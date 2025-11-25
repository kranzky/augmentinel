#include "Platform.h"
#include "View.h"

// Stub implementations for Phase 1
// These will be properly implemented as OpenGL-specific in later phases

View::~View()
{
}

void View::SetPalette(const std::vector<XMFLOAT4> &palette)
{
    for (size_t i = 0; i < palette.size() && i < m_vertexConstants.Palette.size(); ++i)
    {
        m_vertexConstants.Palette[i] = palette[i];
    }
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

bool View::InputAction(Action action)
{
    auto it = m_key_bindings.find(action);
    if (it == m_key_bindings.end())
    {
        return false;
    }

    // Check if any of the bound keys are just pressed or just released
    // Only check edge states (DownEdge) for one-shot actions
    // For continuous movement actions, also check Down state
    for (int key : it->second)
    {
        auto key_state = GetKeyState(key);
        if (key_state == KeyState::DownEdge)
        {
            return true;
        }

        // Also check Down state for continuous movement actions
        if (key_state == KeyState::Down)
        {
            if (action == Action::TurnLeft || action == Action::TurnRight ||
                action == Action::LookUp || action == Action::LookDown ||
                action == Action::Hyperspace || action == Action::U_Turn ||
                action == Action::Transfer || action == Action::Robot ||
                action == Action::Boulder || action == Action::Tree)
            {
                return true;
            }
        }
    }

    // Special handling for VK_ANY - any key pressed (excluding ESC and modifier keys)
    if (std::find(it->second.begin(), it->second.end(), VK_ANY) != it->second.end())
    {
        // Check if any key is pressed, but exclude ESC and modifier keys
        // This allows ALT-TAB, screenshots, etc. without triggering game actions
        for (const auto &pair : m_keys)
        {
            int key = pair.first;
            // Skip ESC and modifier keys (shift, ctrl, alt, gui/command)
            if (key == VK_ESCAPE ||
                key == VK_LSHIFT || key == VK_RSHIFT ||
                key == VK_LCONTROL || key == VK_RCONTROL ||
                key == VK_LALT || key == VK_RALT ||
                key == VK_LGUI || key == VK_RGUI)
            {
                continue;
            }

            if (pair.second == KeyState::Down || pair.second == KeyState::DownEdge)
            {
                return true;
            }
        }
    }

    return false;
}

void View::OutputAction(Action action)
{
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

bool View::IsVR() const
{
    return false;
}

bool View::IsSuspended() const
{
    return false;
}

void View::SetVerticalFOV(float fov)
{
}

void View::OnResize(uint32_t rt_width, uint32_t rt_height)
{
}

void View::SetInputBindings(const std::vector<ActionBinding> &bindings)
{
    m_key_bindings.clear();
    for (const auto &binding : bindings)
    {
        m_key_bindings[binding.action] = binding.virt_keys;
    }
}

void View::PollInputBindings(const std::vector<ActionBinding> &bindings)
{
}

void View::ResetHMD(bool reset)
{
}

void View::BeginScene()
{
}

float View::GetEffect(ViewEffect effect) const
{
    switch (effect)
    {
    case ViewEffect::Dissolve:
        return m_pixelConstants.view_dissolve;
    case ViewEffect::Desaturate:
        return m_pixelConstants.view_desaturate;
    case ViewEffect::Fade:
        return m_pixelConstants.view_fade;
    case ViewEffect::ZFade:
        return m_vertexConstants.z_fade;
    case ViewEffect::FogDensity:
        return m_vertexConstants.fog_density;
    default:
        return 0.0f;
    }
}

void View::SetEffect(ViewEffect effect, float value)
{
    switch (effect)
    {
    case ViewEffect::Dissolve:
        m_pixelConstants.view_dissolve = value;
        break;
    case ViewEffect::Desaturate:
        m_pixelConstants.view_desaturate = value;
        break;
    case ViewEffect::Fade:
        m_pixelConstants.view_fade = value;
        break;
    case ViewEffect::ZFade:
        m_vertexConstants.z_fade = value;
        break;
    case ViewEffect::FogDensity:
        m_vertexConstants.fog_density = value;
        break;
    }
}

bool View::TransitionEffect(ViewEffect effect, float target_value, float elapsed, float total_fade_time)
{
    // Check if we need to start a new transition or update an existing one
    auto it = m_transitions.find(effect);

    if (it == m_transitions.end() || it->second.target_value != target_value)
    {
        // Start a new transition
        TransitionState state;
        state.start_value = GetEffect(effect);
        state.target_value = target_value;
        state.elapsed_time = 0.0f;
        state.total_time = total_fade_time;
        m_transitions[effect] = state;
        it = m_transitions.find(effect);
    }

    // Update elapsed time
    it->second.elapsed_time += elapsed;

    // Check if transition is complete
    if (it->second.elapsed_time >= it->second.total_time)
    {
        SetEffect(effect, target_value);
        m_transitions.erase(it);
        return true;
    }

    // Linear interpolation based on elapsed time
    float t = it->second.elapsed_time / it->second.total_time;
    float new_value = it->second.start_value + (it->second.target_value - it->second.start_value) * t;

    SetEffect(effect, new_value);
    return false;
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

void View::SetPitchLimits(float min_pitch, float max_pitch)
{
    m_camera.SetPitchLimits(min_pitch, max_pitch);
}

void View::MouseMove(int x, int y)
{
    if (!m_freelook)
    {
        return;
    }

    // SDL relative mouse mode provides raw hardware deltas (mickeys), which are much
    // larger than the screen-space pixel deltas used by the Windows version.
    // Scale down to approximate Windows behavior.
    constexpr float SDL_RAW_DELTA_SCALE = 1000.0f;

    // Convert mouse delta to camera rotation
    // x = horizontal movement (yaw), y = vertical movement (pitch)
    m_camera.Yaw((x / SDL_RAW_DELTA_SCALE) / m_mouse_divider);
    m_camera.Pitch((y / SDL_RAW_DELTA_SCALE) / m_mouse_divider * (m_invert_mouse ? -1 : 1));
}

void View::UpdateKey(int virtKey, KeyState state)
{
    m_keys[virtKey] = state;
}

KeyState View::GetKeyState(int key)
{
    auto it = m_keys.find(key);
    return it != m_keys.end() ? it->second : KeyState::Up;
}

bool View::AnyKeyPressed()
{
    for (const auto &pair : m_keys)
    {
        if (pair.second == KeyState::Down || pair.second == KeyState::DownEdge)
        {
            return true;
        }
    }
    return false;
}

void View::ProcessDebugKeys()
{
}

void View::ProcessKeyEdges()
{
    // Convert edge states to sustained states
    for (auto &pair : m_keys)
    {
        if (pair.second == KeyState::DownEdge)
        {
            pair.second = KeyState::Down;
        }
        else if (pair.second == KeyState::UpEdge)
        {
            pair.second = KeyState::Up;
        }
    }

    // Remove keys that are in Up state to keep map clean
    for (auto it = m_keys.begin(); it != m_keys.end();)
    {
        if (it->second == KeyState::Up)
        {
            it = m_keys.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void View::ReleaseKeys()
{
    m_keys.clear();
}

void View::DrawModel(Model &model, const Model &linkedModel)
{
    // Stub for Phase 1 - will be implemented in OpenGLRenderer
}

void View::DrawControllers()
{
    // Stub for VR - not used in Phase 1
}
