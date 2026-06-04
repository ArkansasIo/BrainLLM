#include "voice_audio_system.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace BrainLLM {

VoiceAudioSystem::VoiceAudioSystem() = default;

void VoiceAudioSystem::set_voice_profile(const VoiceProfile& profile) {
    profile_ = profile;
}

VoiceProfile VoiceAudioSystem::get_voice_profile() const {
    return profile_;
}

AudioAsset VoiceAudioSystem::create_tone_asset(const std::string& asset_id,
                                               const std::string& file_path,
                                               float frequency_hz,
                                               int duration_ms) const {
    AudioAsset asset;
    asset.asset_id = asset_id;
    asset.file_path = file_path;
    asset.sample_rate = 44100;
    asset.duration_ms = duration_ms;
    asset.description = "Generated woman voice placeholder tone";

    write_wav_tone(file_path, frequency_hz, duration_ms);
    return asset;
}

bool VoiceAudioSystem::write_wav_tone(const std::string& file_path,
                                      float frequency_hz,
                                      int duration_ms) const {
    constexpr int sample_rate = 44100;
    constexpr int bits_per_sample = 16;
    constexpr int channels = 1;
    const int sample_count = (sample_rate * duration_ms) / 1000;
    const int byte_rate = sample_rate * channels * bits_per_sample / 8;
    const int data_size = sample_count * channels * bits_per_sample / 8;

    std::vector<unsigned char> bytes;
    bytes.reserve(44 + data_size);

    bytes.insert(bytes.end(), {'R', 'I', 'F', 'F'});
    write_u32(bytes, 36 + data_size);
    bytes.insert(bytes.end(), {'W', 'A', 'V', 'E'});
    bytes.insert(bytes.end(), {'f', 'm', 't', ' '});
    write_u32(bytes, 16);
    write_u16(bytes, 1);
    write_u16(bytes, channels);
    write_u32(bytes, sample_rate);
    write_u32(bytes, byte_rate);
    write_u16(bytes, channels * bits_per_sample / 8);
    write_u16(bytes, bits_per_sample);
    bytes.insert(bytes.end(), {'d', 'a', 't', 'a'});
    write_u32(bytes, data_size);

    for (int i = 0; i < sample_count; ++i) {
        const double t = static_cast<double>(i) / sample_rate;
        const double envelope = std::min(1.0, static_cast<double>(i) / 600.0) *
                                std::min(1.0, static_cast<double>(sample_count - i) / 600.0);
        const short sample = static_cast<short>(
            std::sin(2.0 * 3.14159265358979323846 * frequency_hz * t) *
            envelope * 12000.0);
        write_u16(bytes, static_cast<unsigned short>(sample));
    }

    std::filesystem::path path(file_path);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write(reinterpret_cast<const char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
    return true;
}

std::string VoiceAudioSystem::synthesize_to_text_prompt(const std::string& text) const {
    std::ostringstream prompt;
    prompt << "VoiceProfile(" << profile_.display_name
           << ", " << profile_.language
           << ", pitch=" << profile_.pitch_hz
           << "Hz, rate=" << profile_.speaking_rate
           << ", warmth=" << profile_.warmth
           << "): " << text;
    return prompt.str();
}

std::vector<AudioAsset> VoiceAudioSystem::default_woman_voice_assets() const {
    return {
        {"woman_startup", "assets/audio/woman/startup_tone.wav", 44100, 750,
         "Startup tone for woman voice profile"},
        {"woman_notify", "assets/audio/woman/notify_tone.wav", 44100, 350,
         "Notification tone for woman voice profile"}
    };
}

void VoiceAudioSystem::write_u16(std::vector<unsigned char>& bytes, unsigned short value) {
    bytes.push_back(static_cast<unsigned char>(value & 0xff));
    bytes.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
}

void VoiceAudioSystem::write_u32(std::vector<unsigned char>& bytes, unsigned int value) {
    bytes.push_back(static_cast<unsigned char>(value & 0xff));
    bytes.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    bytes.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
    bytes.push_back(static_cast<unsigned char>((value >> 24) & 0xff));
}

} // namespace BrainLLM
