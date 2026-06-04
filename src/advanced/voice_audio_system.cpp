#include "voice_audio_system.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdio>

namespace BrainLLM {

// ── Preset factory ────────────────────────────────────────────────────────────
VoiceProfile make_voice_preset(VoicePersonality p)
{
    VoiceProfile vp;
    vp.personality = p;
    vp.gender      = VoiceGender::Woman;
    switch (p) {
        case VoicePersonality::ARIA:
            vp.profile_id    = "aria";
            vp.display_name  = "ARIA — AI Intelligence";
            vp.pitch_hz      = 195.0f;
            vp.speaking_rate = 1.05f;
            vp.warmth        = 0.80f;
            vp.volume        = 100.0f;
            vp.language      = "en-US";
            break;
        case VoicePersonality::NOVA:
            vp.profile_id    = "nova";
            vp.display_name  = "NOVA — Analytical";
            vp.pitch_hz      = 210.0f;
            vp.speaking_rate = 1.20f;
            vp.warmth        = 0.50f;
            vp.volume        = 100.0f;
            vp.language      = "en-US";
            break;
        case VoicePersonality::SAGE:
            vp.profile_id    = "sage";
            vp.display_name  = "SAGE — Authoritative";
            vp.pitch_hz      = 175.0f;
            vp.speaking_rate = 0.92f;
            vp.warmth        = 0.90f;
            vp.volume        = 100.0f;
            vp.language      = "en-US";
            break;
        case VoicePersonality::ECHO:
            vp.profile_id    = "echo";
            vp.display_name  = "ECHO — Gentle";
            vp.pitch_hz      = 220.0f;
            vp.speaking_rate = 0.88f;
            vp.warmth        = 0.95f;
            vp.volume        = 90.0f;
            vp.language      = "en-US";
            break;
        default:
            vp.profile_id    = "custom";
            vp.display_name  = "Custom Voice";
            break;
    }
    return vp;
}

// ── Constructor ───────────────────────────────────────────────────────────────
VoiceAudioSystem::VoiceAudioSystem()
{
    profile_ = make_voice_preset(VoicePersonality::ARIA);
}

void VoiceAudioSystem::set_voice_profile(const VoiceProfile& p) { profile_ = p; }
VoiceProfile VoiceAudioSystem::get_voice_profile() const        { return profile_; }

void VoiceAudioSystem::apply_personality(VoicePersonality p)
{
    const VoiceProfile preset = make_voice_preset(p);
    // Keep sapi_voice_name if the user set it explicitly
    const std::string saved_name = profile_.sapi_voice_name;
    profile_ = preset;
    if (!saved_name.empty()) profile_.sapi_voice_name = saved_name;
}

// ── List installed SAPI voices ────────────────────────────────────────────────
std::vector<InstalledVoice> VoiceAudioSystem::list_installed_voices() const
{
    std::vector<InstalledVoice> voices;
#ifdef _WIN32
    const std::string script =
        "Add-Type -AssemblyName System.Speech\n"
        "$synth = New-Object System.Speech.Synthesis.SpeechSynthesizer\n"
        "foreach ($v in $synth.GetInstalledVoices()) {\n"
        "  $i = $v.VoiceInfo\n"
        "  Write-Output ($i.Name + '|' + $i.Gender + '|' + $i.Culture.Name)\n"
        "}\n";

    std::string output;
    run_powershell_script(script, &output);

    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
            line.pop_back();
        if (line.empty()) continue;
        InstalledVoice iv;
        auto p1 = line.find('|');
        auto p2 = line.find('|', p1 + 1);
        if (p1 == std::string::npos) { iv.name = line; }
        else {
            iv.name     = line.substr(0, p1);
            iv.gender   = (p2 == std::string::npos) ? line.substr(p1 + 1) : line.substr(p1 + 1, p2 - p1 - 1);
            iv.language = (p2 == std::string::npos) ? "" : line.substr(p2 + 1);
        }
        voices.push_back(iv);
    }
#endif
    return voices;
}

// ── SSML builder ─────────────────────────────────────────────────────────────
// Maps profile pitch/rate/warmth to SSML prosody.
// pitch_hz 175="-5%" .. 220="+10%"   (reference 190 Hz ≈ average female speaking)
// rate 0.8="-20%" .. 1.2="+20%"
// warmth controls emphasis: high warmth adds soft breaks and slightly slower
std::string VoiceAudioSystem::build_ssml(const std::string& text) const
{
    // pitch offset from reference 190 Hz
    float pitch_pct = (profile_.pitch_hz - 190.0f) / 190.0f * 100.0f;
    int   pitch_int = static_cast<int>(std::round(pitch_pct));

    float rate_pct  = (profile_.speaking_rate - 1.0f) * 100.0f;
    int   rate_int  = static_cast<int>(std::round(rate_pct));

    // volume: direct 0-100
    int vol = static_cast<int>(profile_.volume);
    vol = std::max(0, std::min(100, vol));

    std::string pitch_str = (pitch_int >= 0)
        ? "+" + std::to_string(pitch_int) + "%"
        :       std::to_string(pitch_int) + "%";

    std::string rate_str = (rate_int >= 0)
        ? "+" + std::to_string(rate_int) + "%"
        :       std::to_string(rate_int) + "%";

    // Warmth: add subtle emphasis on key words and a tiny leading break
    std::string warmth_break = "";
    if (profile_.warmth > 0.6f) {
        warmth_break = "<break time=\"80ms\"/>";
    }

    std::ostringstream ssml;
    ssml << "<speak version=\"1.0\" "
         << "xmlns=\"http://www.w3.org/2001/10/synthesis\" "
         << "xml:lang=\"" << profile_.language << "\">"
         << "<prosody pitch=\"" << pitch_str << "\" "
         << "rate=\"" << rate_str << "\" "
         << "volume=\"" << vol << "\">"
         << warmth_break
         << escape_ps(text)          // plain text inside SSML
         << "</prosody>"
         << "</speak>";
    return ssml.str();
}

// ── PowerShell script builder ─────────────────────────────────────────────────
std::string VoiceAudioSystem::build_speak_script(const std::string& text,
                                                  bool to_wav,
                                                  const std::string& wav_path) const
{
    const int rate_val = std::max(-10, std::min(10,
        static_cast<int>((profile_.speaking_rate - 1.0f) * 10.0f)));
    const int vol_val  = static_cast<int>(profile_.volume);

    std::ostringstream s;
    s << "Add-Type -AssemblyName System.Speech\n"
      << "$synth = New-Object System.Speech.Synthesis.SpeechSynthesizer\n"
      << "$synth.Rate = " << rate_val << "\n"
      << "$synth.Volume = " << vol_val << "\n";

    // Voice selection: named override > gender hint
    if (!profile_.sapi_voice_name.empty()) {
        s << "try { $synth.SelectVoice('" << escape_ps(profile_.sapi_voice_name) << "') }"
          << " catch { try { $synth.SelectVoiceByHints("
          << "[System.Speech.Synthesis.VoiceGender]::Female) } catch {} }\n";
    } else {
        s << "try { $synth.SelectVoiceByHints("
          << "[System.Speech.Synthesis.VoiceGender]::Female) } catch {}\n";
    }

    if (to_wav) {
        s << "$synth.SetOutputToWaveFile('" << escape_ps(wav_path) << "')\n";
    }

    // Try SSML first; fall back to plain text if SSML fails
    const std::string ssml = build_ssml(text);
    s << "try {\n"
      << "  $synth.SpeakSsml('" << escape_ps(ssml) << "')\n"
      << "} catch {\n"
      << "  $synth.Speak('" << escape_ps(text) << "')\n"
      << "}\n"
      << "$synth.Dispose()\n";

    return s.str();
}

// ── speak_text ────────────────────────────────────────────────────────────────
bool VoiceAudioSystem::is_platform_speech_available() const
{
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

TextToSpeechResult VoiceAudioSystem::speak_text(const std::string& text) const
{
    if (text.empty())
        return {false, "", "", "No text provided."};
#ifdef _WIN32
    const std::string script = build_speak_script(text, false);
    const int exit_code = run_powershell_script(script);
    if (exit_code != 0)
        return {false, "", "", "Speech synthesis failed (exit " + std::to_string(exit_code) + ")."};
    return {true, "", "Spoken via Windows SAPI — " + profile_.display_name, ""};
#else
    return {false, "", "", "TTS requires Windows System.Speech."};
#endif
}

TextToSpeechResult VoiceAudioSystem::speak_ssml(const std::string& ssml) const
{
    if (ssml.empty())
        return {false, "", "", "No SSML provided."};
#ifdef _WIN32
    std::ostringstream s;
    s << "Add-Type -AssemblyName System.Speech\n"
      << "$synth = New-Object System.Speech.Synthesis.SpeechSynthesizer\n"
      << "$synth.Volume = " << static_cast<int>(profile_.volume) << "\n"
      << "try { $synth.SelectVoiceByHints([System.Speech.Synthesis.VoiceGender]::Female) } catch {}\n"
      << "try { $synth.SpeakSsml('" << escape_ps(ssml) << "') } catch { }\n"
      << "$synth.Dispose()\n";
    const int exit_code = run_powershell_script(s.str());
    return exit_code == 0
        ? TextToSpeechResult{true, "", "SSML spoken.", ""}
        : TextToSpeechResult{false, "", "", "SSML speech failed."};
#else
    return {false, "", "", "TTS requires Windows System.Speech."};
#endif
}

TextToSpeechResult VoiceAudioSystem::synthesize_to_wav(const std::string& text,
                                                        const std::string& file_path) const
{
    if (text.empty())      return {false, "", "", "No text provided."};
    if (file_path.empty()) return {false, "", "", "No output path provided."};
#ifdef _WIN32
    std::filesystem::path fsp(file_path);
    if (fsp.has_parent_path())
        std::filesystem::create_directories(fsp.parent_path());

    const std::string script = build_speak_script(text, true, fsp.string());
    const int exit_code = run_powershell_script(script);
    if (exit_code != 0 || !std::filesystem::exists(fsp))
        return {false, "", "", "WAV synthesis failed."};
    return {true, fsp.string(), "Synthesized to WAV — " + profile_.display_name, ""};
#else
    return {false, "", "", "WAV TTS requires Windows System.Speech."};
#endif
}

// ── AI personality helpers ────────────────────────────────────────────────────
void VoiceAudioSystem::speak_startup_greeting() const
{
    const std::string greeting =
        "BrainLLM neural interface online. "
        "All systems operational. "
        "I am ready to assist you.";
    speak_text(greeting);
}

void VoiceAudioSystem::speak_processing_tone() const
{
    speak_text("Processing.");
}

void VoiceAudioSystem::speak_response(const std::string& text) const
{
    if (text.empty()) return;
    // Strip bracketed metadata like [Topics: ...] and [Confidence: ...]
    std::string clean = text;
    auto strip = [&](const std::string& tag) {
        size_t pos;
        while ((pos = clean.find(tag)) != std::string::npos) {
            size_t end = clean.find(']', pos);
            if (end != std::string::npos)
                clean.erase(pos, end - pos + 1);
            else
                break;
        }
    };
    strip("[Topics:");
    strip("[Confidence:");
    strip("[AirLLM");
    // Trim leading "BrainLLM response: " prefix for cleaner speech
    const std::string prefix = "BrainLLM response: ";
    if (clean.rfind(prefix, 0) == 0)
        clean = clean.substr(prefix.size());
    speak_text(clean);
}

// ── Speech recognition ────────────────────────────────────────────────────────
SpeechRecognitionResult VoiceAudioSystem::recognize_once(int timeout_seconds) const
{
#ifdef _WIN32
    const int timeout = std::max(1, std::min(60, timeout_seconds));
    std::ostringstream script;
    script << "Add-Type -AssemblyName System.Speech\n"
           << "$rec = New-Object System.Speech.Recognition.SpeechRecognitionEngine\n"
           << "$rec.SetInputToDefaultAudioDevice()\n"
           << "$rec.LoadGrammar((New-Object System.Speech.Recognition.DictationGrammar))\n"
           << "$result = $rec.Recognize([TimeSpan]::FromSeconds(" << timeout << "))\n"
           << "if ($null -eq $result) { exit 2 }\n"
           << "Write-Output ($result.Text + '|confidence=' + $result.Confidence)\n";

    std::string output;
    const int exit_code = run_powershell_script(script.str(), &output);
    if (exit_code != 0 || output.empty())
        return {false, "", 0.0f, "No speech recognized. Check microphone permissions."};

    const std::string marker = "|confidence=";
    const size_t mpos = output.find(marker);
    SpeechRecognitionResult result;
    result.success    = true;
    result.transcript = (mpos == std::string::npos) ? output : output.substr(0, mpos);
    while (!result.transcript.empty() &&
           (result.transcript.back() == '\r' || result.transcript.back() == '\n'))
        result.transcript.pop_back();
    if (mpos != std::string::npos) {
        try { result.confidence = std::stof(output.substr(mpos + marker.size())); }
        catch (...) { result.confidence = 0.0f; }
    }
    return result;
#else
    return {false, "", 0.0f, "Speech recognition requires Windows System.Speech."};
#endif
}

// ── Misc ──────────────────────────────────────────────────────────────────────
std::string VoiceAudioSystem::synthesize_to_text_prompt(const std::string& text) const
{
    std::ostringstream p;
    p << "VoiceProfile(" << profile_.display_name
      << ", " << profile_.language
      << ", pitch=" << profile_.pitch_hz
      << "Hz, rate=" << profile_.speaking_rate
      << ", warmth=" << profile_.warmth
      << "): " << text;
    return p.str();
}

std::vector<AudioAsset> VoiceAudioSystem::default_woman_voice_assets() const
{
    return {
        {"aria_startup", "assets/audio/woman/startup_tone.wav", 44100, 750,
         "Startup tone for ARIA voice profile"},
        {"aria_notify",  "assets/audio/woman/notify_tone.wav",  44100, 350,
         "Notification tone for ARIA voice profile"}
    };
}

AudioAsset VoiceAudioSystem::create_tone_asset(const std::string& asset_id,
                                                const std::string& file_path,
                                                float frequency_hz,
                                                int duration_ms) const
{
    AudioAsset asset;
    asset.asset_id    = asset_id;
    asset.file_path   = file_path;
    asset.sample_rate = 44100;
    asset.duration_ms = duration_ms;
    asset.description = "Generated tone";
    write_wav_tone(file_path, frequency_hz, duration_ms);
    return asset;
}

bool VoiceAudioSystem::write_wav_tone(const std::string& file_path,
                                       float frequency_hz,
                                       int duration_ms) const
{
    constexpr int sample_rate     = 44100;
    constexpr int bits_per_sample = 16;
    constexpr int channels        = 1;
    const int sample_count  = (sample_rate * duration_ms) / 1000;
    const int byte_rate     = sample_rate * channels * bits_per_sample / 8;
    const int data_size     = sample_count * channels * bits_per_sample / 8;

    std::vector<unsigned char> bytes;
    bytes.reserve(44 + data_size);
    bytes.insert(bytes.end(), {'R','I','F','F'});
    write_u32(bytes, 36 + data_size);
    bytes.insert(bytes.end(), {'W','A','V','E'});
    bytes.insert(bytes.end(), {'f','m','t',' '});
    write_u32(bytes, 16);
    write_u16(bytes, 1);
    write_u16(bytes, channels);
    write_u32(bytes, sample_rate);
    write_u32(bytes, byte_rate);
    write_u16(bytes, channels * bits_per_sample / 8);
    write_u16(bytes, bits_per_sample);
    bytes.insert(bytes.end(), {'d','a','t','a'});
    write_u32(bytes, data_size);

    for (int i = 0; i < sample_count; ++i) {
        const double t = static_cast<double>(i) / sample_rate;
        const double env = std::min(1.0, static_cast<double>(i) / 600.0) *
                           std::min(1.0, static_cast<double>(sample_count - i) / 600.0);
        const short sample = static_cast<short>(
            std::sin(2.0 * 3.14159265358979323846 * frequency_hz * t) * env * 12000.0);
        write_u16(bytes, static_cast<unsigned short>(sample));
    }

    std::filesystem::path p(file_path);
    if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());

    std::ofstream f(file_path, std::ios::binary);
    if (!f.is_open()) return false;
    f.write(reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    return true;
}

// ── Private helpers ───────────────────────────────────────────────────────────
std::string VoiceAudioSystem::escape_ps(const std::string& value)
{
    // Escape single quotes for PowerShell single-quoted strings
    std::string out;
    out.reserve(value.size());
    for (char c : value) {
        out += c;
        if (c == '\'') out += '\'';
    }
    return out;
}

int VoiceAudioSystem::run_powershell_script(const std::string& script,
                                             std::string* output)
{
#ifdef _WIN32
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const auto tmp = std::filesystem::temp_directory_path();
    const auto script_path = tmp / ("brainllm_tts_" + std::to_string(now) + ".ps1");
    const auto output_path = tmp / ("brainllm_tts_" + std::to_string(now) + ".out");

    { std::ofstream f(script_path); if (!f) return 1; f << script; }

    std::ostringstream cmd;
    cmd << "powershell -NoProfile -ExecutionPolicy Bypass -File \""
        << script_path.string() << "\"";
    if (output)
        cmd << " > \"" << output_path.string() << "\" 2>&1";

    const int rc = std::system(cmd.str().c_str());

    if (output) {
        std::ifstream f(output_path);
        std::ostringstream buf; buf << f.rdbuf();
        *output = buf.str();
        std::filesystem::remove(output_path);
    }
    std::filesystem::remove(script_path);
    return rc;
#else
    (void)script; (void)output; return 1;
#endif
}

void VoiceAudioSystem::write_u16(std::vector<unsigned char>& b, unsigned short v)
{
    b.push_back(v & 0xff);
    b.push_back((v >> 8) & 0xff);
}

void VoiceAudioSystem::write_u32(std::vector<unsigned char>& b, unsigned int v)
{
    b.push_back(v & 0xff);
    b.push_back((v >> 8) & 0xff);
    b.push_back((v >> 16) & 0xff);
    b.push_back((v >> 24) & 0xff);
}

} // namespace BrainLLM
