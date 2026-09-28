#include "Platform.h"
#include "View.h"

// Actions that repeat for as long as their key is held, rather than once per press.
static bool IsContinuousAction(Action action)
{
    return action == Action::TurnLeft || action == Action::TurnRight ||
           action == Action::LookUp || action == Action::LookDown;
}

// Keys that never count as "any key": Escape has its own meaning, and modifiers are
// pressed for system shortcuts (Alt-Tab, Cmd-Tab, screenshots).
static bool IsExcludedFromAnyKey(int key)
{
    switch (key)
    {
    case SDLK_ESCAPE:
    case SDLK_LSHIFT: case SDLK_RSHIFT:
    case SDLK_LCTRL:  case SDLK_RCTRL:
    case SDLK_LALT:   case SDLK_RALT:
    case SDLK_LGUI:   case SDLK_RGUI:
        return true;
    default:
        return false;
    }
}

View::~View() = default;

void View::SetPalette(const std::vector<XMFLOAT4>& palette)
{
    auto count = std::min(palette.size(), m_vertexConstants.Palette.size());
    std::copy_n(palette.begin(), count, m_vertexConstants.Palette.begin());
}

void View::SetFillColour(int fill_colour_idx)
{
    m_fill_colour_idx = fill_colour_idx;
}

void View::SetFogColour(int fog_colour_idx)
{
    m_vertexConstants.fog_colour_idx = fog_colour_idx;
}

void View::SetMouseSpeed(int percent)
{
    m_mouse_divider = static_cast<float>(DEFAULT_MOUSE_DIVISOR) / (percent * MOUSE_DIVISOR_STEP);
}

void View::EnableFreeLook(bool enable)
{
    m_freelook = enable;
}

XMFLOAT3 View::GetEyePosition() const
{
    XMFLOAT3 pos;
    XMStoreFloat3(&pos, GetEyePositionVector());
    return pos;
}

XMFLOAT3 View::GetViewPosition() const
{
    XMFLOAT3 pos;
    XMStoreFloat3(&pos, GetViewPositionVector());
    return pos;
}

XMFLOAT3 View::GetViewDirection() const
{
    XMFLOAT3 dir;
    XMStoreFloat3(&dir, GetViewDirectionVector());
    return dir;
}

XMFLOAT3 View::GetUpDirection() const
{
    XMFLOAT3 up;
    XMStoreFloat3(&up, GetViewUpVector());
    return up;
}

XMFLOAT3 View::GetCameraPosition() const
{
    return m_camera.GetPosition();
}

XMFLOAT3 View::GetCameraDirection() const
{
    return m_camera.GetDirection();
}

XMFLOAT3 View::GetCameraRotation() const
{
    return m_camera.GetRotations();
}

void View::SetCameraPosition(XMFLOAT3 pos)
{
    m_camera.SetPosition(pos);
}

void View::SetCameraRotation(XMFLOAT3 rot)
{
    m_camera.SetRotation(rot);
}

void View::SetPitchLimits(float min_pitch, float max_pitch)
{
    m_camera.SetPitchLimits(min_pitch, max_pitch);
}

bool View::IsVR() const
{
    return false;
}

bool View::IsSuspended() const
{
    return false;
}

void View::SetVerticalFOV(float /*fov*/)
{
}

void View::OnResize(uint32_t /*rt_width*/, uint32_t /*rt_height*/)
{
}

void View::BeginScene()
{
}

void View::DrawModel(Model& /*model*/, const Model& /*linkedModel*/)
{
}

void View::DrawControllers()
{
    // VR only.
}

void View::ResetHMD(bool /*reset*/)
{
    // VR only.
}

void View::OutputAction(Action /*action*/)
{
    // VR only (haptics).
}

void View::PollInputBindings(const std::vector<ActionBinding>& /*bindings*/)
{
    // VR only.
}

////////////////////////////////////////////////////////////////////////////////
// Effects

float View::GetEffect(ViewEffect effect) const
{
    switch (effect)
    {
    case ViewEffect::Dissolve:   return m_pixelConstants.view_dissolve;
    case ViewEffect::Desaturate: return m_pixelConstants.view_desaturate;
    case ViewEffect::Fade:       return m_pixelConstants.view_fade;
    case ViewEffect::ZFade:      return m_vertexConstants.z_fade;
    case ViewEffect::FogDensity: return m_vertexConstants.fog_density;
    }
    return 0.0f;
}

void View::SetEffect(ViewEffect effect, float value)
{
    switch (effect)
    {
    case ViewEffect::Dissolve:   m_pixelConstants.view_dissolve = value; break;
    case ViewEffect::Desaturate: m_pixelConstants.view_desaturate = value; break;
    case ViewEffect::Fade:       m_pixelConstants.view_fade = value; break;
    case ViewEffect::ZFade:      m_vertexConstants.z_fade = value; break;
    case ViewEffect::FogDensity: m_vertexConstants.fog_density = value; break;
    }
}

// Move an effect towards a target value at a rate that covers the full 0-1 range in
// total_fade_time seconds. Returns true once the effect is already at the target, so
// callers can wait on it every frame without any extra state.
bool View::TransitionEffect(ViewEffect effect, float target_value, float elapsed, float total_fade_time)
{
    auto current_value = GetEffect(effect);
    auto value_change = elapsed / total_fade_time;

    if (target_value > current_value)
        SetEffect(effect, std::min(current_value + value_change, target_value));
    else
        SetEffect(effect, std::max(current_value - value_change, target_value));

    return current_value == target_value;
}

void View::EnableAnimatedNoise(bool enable)
{
    m_noise_enabled = enable;
}

bool View::PixelShaderEffectsActive() const
{
    return m_pixelConstants.view_dissolve > 0.0f ||
           m_pixelConstants.view_desaturate > 0.0f ||
           m_pixelConstants.view_fade > 0.0f;
}

////////////////////////////////////////////////////////////////////////////////
// Input

void View::MouseMove(int x, int y)
{
    if (!m_freelook)
        return;

    // SDL relative mouse deltas are much larger than the window-space deltas the
    // original Windows version was tuned for, so scale them down to match.
    constexpr float SDL_RAW_DELTA_SCALE = 1000.0f;

    m_camera.Yaw((x / SDL_RAW_DELTA_SCALE) / m_mouse_divider);
    m_camera.Pitch((y / SDL_RAW_DELTA_SCALE) / m_mouse_divider * (m_invert_mouse ? -1 : 1));
}

void View::UpdateKey(int virtKey, KeyState state)
{
    if (state == KeyState::DownEdge)
    {
        m_keys[virtKey] = KeyState::DownEdge;
        return;
    }

    // A release before the press was consumed is kept until the end of the frame,
    // so a tap whose press and release arrive together still triggers its action.
    auto it = m_keys.find(virtKey);
    if (it != m_keys.end() && it->second == KeyState::DownEdge)
        it->second = KeyState::UpEdge;
    else if (it != m_keys.end())
        m_keys.erase(it);
}

void View::EndInputFrame()
{
    // Drop taps that nothing consumed this frame, as a released key would be.
    for (auto it = m_keys.begin(); it != m_keys.end();)
        it = (it->second == KeyState::UpEdge) ? m_keys.erase(it) : std::next(it);
}

void View::ReleaseKeys()
{
    m_keys.clear();
}

bool View::ConsumeKeyPress(int key)
{
    auto it = m_keys.find(key);
    if (it == m_keys.end())
        return false;

    switch (it->second)
    {
    case KeyState::DownEdge:
        it->second = KeyState::Down;
        return true;
    case KeyState::UpEdge:
        m_keys.erase(it);
        return true;
    default:
        return false;
    }
}

bool View::IsKeyActive(int key) const
{
    return m_keys.find(key) != m_keys.end();
}

bool View::ConsumeAnyKeyPress()
{
    for (auto& [key, state] : m_keys)
    {
        if (!IsExcludedFromAnyKey(key) && (state == KeyState::DownEdge || state == KeyState::UpEdge))
            return ConsumeKeyPress(key);
    }
    return false;
}

void View::SetInputBindings(const std::vector<ActionBinding>& bindings)
{
    m_key_bindings.clear();
    for (const auto& binding : bindings)
        m_key_bindings[binding.action] = binding.virt_keys;
}

// Continuous actions are active while any bound key is held. Other actions trigger
// once per press, consuming it so that no other check in the frame sees it again.
bool View::InputAction(Action action)
{
    auto it = m_key_bindings.find(action);
    if (it == m_key_bindings.end())
        return false;

    for (auto key : it->second)
    {
        if (key == VK_ANY)
        {
            if (ConsumeAnyKeyPress())
                return true;
        }
        else if (IsContinuousAction(action) ? IsKeyActive(key) : ConsumeKeyPress(key))
        {
            return true;
        }
    }

    return false;
}

void View::ProcessDebugKeys()
{
}
