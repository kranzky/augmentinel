#include "Platform.h"
#include "View.h"

// Stub implementations for Phase 1
// These will be properly implemented as OpenGL-specific in later phases

View::~View() {
}

void View::SetPalette(const std::vector<XMFLOAT4>& palette) {
    for (size_t i = 0; i < palette.size() && i < m_vertexConstants.Palette.size(); ++i) {
        m_vertexConstants.Palette[i] = palette[i];
    }
}

void View::SetFillColour(int fill_colour_idx) {
    m_fill_colour_idx = fill_colour_idx;
}

void View::SetFogColour(int fog_colour_idx) {
    m_vertexConstants.fog_colour_idx = fog_colour_idx;
}

void View::SetMouseSpeed(int percent) {
    m_mouse_divider = static_cast<float>(DEFAULT_MOUSE_DIVISOR) / (percent * MOUSE_DIVISOR_STEP);
}

void View::EnableFreeLook(bool enable) {
    m_freelook = enable;
}

bool View::InputAction(Action action) {
    auto it = m_key_bindings.find(action);
    if (it == m_key_bindings.end()) {
        return false;
    }

    // Check if any of the bound keys are down or just pressed
    for (int key : it->second) {
        auto key_state = GetKeyState(key);
        if (key_state == KeyState::Down || key_state == KeyState::DownEdge) {
            return true;
        }
    }

    // Special handling for VK_ANY - any key pressed
    if (std::find(it->second.begin(), it->second.end(), VK_ANY) != it->second.end()) {
        return AnyKeyPressed();
    }

    return false;
}

void View::OutputAction(Action action) {
}

XMFLOAT3 View::GetEyePosition() const {
    XMFLOAT3 pos;
    XMStoreFloat3(&pos, GetEyePositionVector());
    return pos;
}

XMFLOAT3 View::GetViewPosition() const {
    XMFLOAT3 pos;
    XMStoreFloat3(&pos, GetViewPositionVector());
    return pos;
}

XMFLOAT3 View::GetViewDirection() const {
    XMFLOAT3 dir;
    XMStoreFloat3(&dir, GetViewDirectionVector());
    return dir;
}

XMFLOAT3 View::GetUpDirection() const {
    XMFLOAT3 up;
    XMStoreFloat3(&up, GetViewUpVector());
    return up;
}

XMFLOAT3 View::GetCameraPosition() const {
    return m_camera.GetPosition();
}

XMFLOAT3 View::GetCameraDirection() const {
    return m_camera.GetDirection();
}

XMFLOAT3 View::GetCameraRotation() const {
    return m_camera.GetRotations();
}

void View::SetCameraPosition(XMFLOAT3 pos) {
    m_camera.SetPosition(pos);
}

void View::SetCameraRotation(XMFLOAT3 rot) {
    m_camera.SetRotation(rot);
}

bool View::IsVR() const {
    return false;
}

bool View::IsSuspended() const {
    return false;
}

void View::SetVerticalFOV(float fov) {
}

void View::OnResize(uint32_t rt_width, uint32_t rt_height) {
}

void View::SetInputBindings(const std::vector<ActionBinding>& bindings) {
    m_key_bindings.clear();
    for (const auto& binding : bindings) {
        m_key_bindings[binding.action] = binding.virt_keys;
    }
}

void View::PollInputBindings(const std::vector<ActionBinding>& bindings) {
}

void View::ResetHMD(bool reset) {
}

void View::BeginScene() {
}

float View::GetEffect(ViewEffect effect) const {
    switch (effect) {
    case ViewEffect::Dissolve:   return m_pixelConstants.view_dissolve;
    case ViewEffect::Desaturate: return m_pixelConstants.view_desaturate;
    case ViewEffect::Fade:       return m_pixelConstants.view_fade;
    case ViewEffect::ZFade:      return m_vertexConstants.z_fade;
    case ViewEffect::FogDensity: return m_vertexConstants.fog_density;
    default: return 0.0f;
    }
}

void View::SetEffect(ViewEffect effect, float value) {
    switch (effect) {
    case ViewEffect::Dissolve:   m_pixelConstants.view_dissolve = value; break;
    case ViewEffect::Desaturate: m_pixelConstants.view_desaturate = value; break;
    case ViewEffect::Fade:       m_pixelConstants.view_fade = value; break;
    case ViewEffect::ZFade:      m_vertexConstants.z_fade = value; break;
    case ViewEffect::FogDensity: m_vertexConstants.fog_density = value; break;
    }
}

bool View::TransitionEffect(ViewEffect effect, float target_value, float elapsed, float total_fade_time) {
    auto current_value = GetEffect(effect);
    auto delta = target_value - current_value;

    if (std::abs(delta) < 0.001f) {
        SetEffect(effect, target_value);
        return true;
    }

    auto step = (delta / total_fade_time) * elapsed;
    auto new_value = current_value + step;

    if ((delta > 0 && new_value >= target_value) || (delta < 0 && new_value <= target_value)) {
        SetEffect(effect, target_value);
        return true;
    }

    SetEffect(effect, new_value);
    return false;
}

void View::EnableAnimatedNoise(bool enable) {
    m_noise_enabled = enable;
}

bool View::PixelShaderEffectsActive() const {
    return m_pixelConstants.view_dissolve > 0.0f ||
           m_pixelConstants.view_desaturate > 0.0f ||
           m_pixelConstants.view_fade > 0.0f;
}

void View::SetPitchLimits(float min_pitch, float max_pitch) {
    m_camera.SetPitchLimits(min_pitch, max_pitch);
}

void View::MouseMove(int x, int y) {
    if (!m_freelook) {
        return;
    }

    // Convert mouse delta to camera rotation
    // SDL mouse motion is much more sensitive than Win32, so we apply an additional multiplier
    // x = horizontal movement (yaw), y = vertical movement (pitch)
    constexpr float SDL_MOUSE_SENSITIVITY_MULTIPLIER = 100.0f;
    float yaw = static_cast<float>(x) / (m_mouse_divider * SDL_MOUSE_SENSITIVITY_MULTIPLIER);
    float pitch = static_cast<float>(y) / (m_mouse_divider * SDL_MOUSE_SENSITIVITY_MULTIPLIER);

    // Apply mouse inversion if enabled
    if (m_invert_mouse) {
        pitch = -pitch;
    }

    // Update camera rotation
    m_camera.Yaw(yaw);
    m_camera.Pitch(pitch);
}

void View::UpdateKey(int virtKey, KeyState state) {
    m_keys[virtKey] = state;
}

KeyState View::GetKeyState(int key) {
    auto it = m_keys.find(key);
    return it != m_keys.end() ? it->second : KeyState::Up;
}

bool View::AnyKeyPressed() {
    for (const auto& pair : m_keys) {
        if (pair.second == KeyState::Down || pair.second == KeyState::DownEdge) {
            return true;
        }
    }
    return false;
}

void View::ProcessDebugKeys() {
}

void View::ProcessKeyEdges() {
    // Convert edge states to sustained states
    for (auto& pair : m_keys) {
        if (pair.second == KeyState::DownEdge) {
            pair.second = KeyState::Down;
        } else if (pair.second == KeyState::UpEdge) {
            pair.second = KeyState::Up;
        }
    }

    // Remove keys that are in Up state to keep map clean
    for (auto it = m_keys.begin(); it != m_keys.end(); ) {
        if (it->second == KeyState::Up) {
            it = m_keys.erase(it);
        } else {
            ++it;
        }
    }
}

void View::ReleaseKeys() {
    m_keys.clear();
}

void View::DrawModel(Model& model, const Model& linkedModel) {
    // Stub for Phase 1 - will be implemented in OpenGLRenderer
}

void View::DrawControllers() {
    // Stub for VR - not used in Phase 1
}
