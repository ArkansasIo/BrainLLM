#pragma once

#include <string>
#include <vector>

namespace BrainLLM {

enum class VoiceGender {
    Neutral,
    Woman,
    Man
};

struct VoiceProfile {
    std::string profile_id = "woman_default";
    std::string display_name = "BrainLLM Woman Voice";
    VoiceGender gender = VoiceGender::Woman;
    float pitch_hz = 220.0f;
    float speaking_rate = 1.0f;
    float warmth = 0.75f;
    std::string language = "en-US";
};

struct AudioAsset {
    std::string asset_id;
    std::string file_path;
    int sample_rate = 44100;
    int duration_ms = 0;
    std::string description;
};

class VoiceAudioSystem {
public:
    VoiceAudioSystem();
    ~VoiceAudioSystem() = default;

    void set_voice_profile(const VoiceProfile& profile);
    VoiceProfile get_voice_profile() const;

    AudioAsset create_tone_asset(const std::string& asset_id,
                                 const std::string& file_path,
                                 float frequency_hz,
                                 int duration_ms) const;
    bool write_wav_tone(const std::string& file_path,
                        float frequency_hz,
                        int duration_ms) const;

    std::string synthesize_to_text_prompt(const std::string& text) const;
    std::vector<AudioAsset> default_woman_voice_assets() const;

private:
    VoiceProfile profile_;

    static void write_u16(std::vector<unsigned char>& bytes, unsigned short value);
    static void write_u32(std::vector<unsigned char>& bytes, unsigned int value);
};

} // namespace BrainLLM
