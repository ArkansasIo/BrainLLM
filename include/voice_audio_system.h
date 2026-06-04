#pragma once

#include <string>
#include <vector>
#include <functional>

namespace BrainLLM {

enum class VoiceGender {
    Neutral,
    Woman,
    Man
};

// ── Voice personality presets ────────────────────────────────────────────────
enum class VoicePersonality {
    ARIA,       // Calm, intelligent, slightly warm — default AI female
    NOVA,       // Crisp, fast, analytical — JARVIS style
    SAGE,       // Warm, measured, authoritative
    ECHO,       // Softer, breathy, gentle
    Custom
};

struct VoiceProfile {
    std::string     profile_id   = "aria_default";
    std::string     display_name = "ARIA — BrainLLM AI Voice";
    VoiceGender     gender       = VoiceGender::Woman;
    VoicePersonality personality = VoicePersonality::ARIA;
    float pitch_hz       = 195.0f;   // slightly raised for AI female
    float speaking_rate  = 1.05f;    // just slightly faster than natural
    float warmth         = 0.80f;    // 0=robotic .. 1=warm
    float volume         = 100.0f;   // 0-100
    std::string language = "en-US";
    // Windows SAPI voice name override (empty = auto-select by gender)
    std::string sapi_voice_name;
    // SSML extra prosody tags injected before text (optional)
    std::string ssml_prefix;
};

// ── Preset factory ────────────────────────────────────────────────────────────
VoiceProfile make_voice_preset(VoicePersonality p);

struct AudioAsset {
    std::string asset_id;
    std::string file_path;
    int  sample_rate  = 44100;
    int  duration_ms  = 0;
    std::string description;
};

struct SpeechRecognitionResult {
    bool        success    = false;
    std::string transcript;
    float       confidence = 0.0f;
    std::string error;
};

struct TextToSpeechResult {
    bool        success     = false;
    std::string output_path;
    std::string message;
    std::string error;
};

// ── Installed SAPI voice descriptor ─────────────────────────────────────────
struct InstalledVoice {
    std::string name;
    std::string gender;   // "Female" / "Male"
    std::string language;
};

// ── VoiceAudioSystem ─────────────────────────────────────────────────────────
class VoiceAudioSystem {
public:
    VoiceAudioSystem();
    ~VoiceAudioSystem() = default;

    // Profile management
    void         set_voice_profile(const VoiceProfile& profile);
    VoiceProfile get_voice_profile() const;
    void         apply_personality(VoicePersonality p);

    // Voice discovery
    std::vector<InstalledVoice> list_installed_voices() const;

    // Tone / asset generation
    AudioAsset create_tone_asset(const std::string& asset_id,
                                  const std::string& file_path,
                                  float frequency_hz,
                                  int duration_ms) const;
    bool write_wav_tone(const std::string& file_path,
                        float frequency_hz,
                        int duration_ms) const;

    // Platform detection
    bool is_platform_speech_available() const;

    // Text-to-speech
    TextToSpeechResult speak_text(const std::string& text) const;
    TextToSpeechResult speak_ssml(const std::string& ssml) const;   // raw SSML
    TextToSpeechResult synthesize_to_wav(const std::string& text,
                                          const std::string& file_path) const;

    // Quick helpers for AI personality phrases
    void speak_startup_greeting()   const;
    void speak_processing_tone()    const;
    void speak_response(const std::string& text) const; // formatted AI-style

    // Speech recognition
    SpeechRecognitionResult recognize_once(int timeout_seconds = 6) const;

    // Misc
    std::string synthesize_to_text_prompt(const std::string& text) const;
    std::vector<AudioAsset> default_woman_voice_assets() const;

private:
    VoiceProfile profile_;

    // ── SSML helpers ──────────────────────────────────────────────────────────
    std::string build_ssml(const std::string& text) const;
    std::string build_speak_script(const std::string& text,
                                    bool to_wav,
                                    const std::string& wav_path = "") const;

    static std::string escape_ps(const std::string& value);
    static int  run_powershell_script(const std::string& script,
                                      std::string* output = nullptr);
    static void write_u16(std::vector<unsigned char>& bytes, unsigned short value);
    static void write_u32(std::vector<unsigned char>& bytes, unsigned int  value);
};

} // namespace BrainLLM
