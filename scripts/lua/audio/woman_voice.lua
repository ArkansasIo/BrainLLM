local audio = require("scripts.lua.subscripts.audio_helpers")

local woman_voice = {}

local emotion_presets = {
    neutral = { pitch_hz = 220.0, speaking_rate = 1.0, warmth = 0.75 },
    calm = { pitch_hz = 205.0, speaking_rate = 0.9, warmth = 0.9 },
    alert = { pitch_hz = 245.0, speaking_rate = 1.15, warmth = 0.65 },
    friendly = { pitch_hz = 232.0, speaking_rate = 1.05, warmth = 0.95 },
    serious = { pitch_hz = 198.0, speaking_rate = 0.92, warmth = 0.55 },
    excited = { pitch_hz = 260.0, speaking_rate = 1.25, warmth = 0.85 }
}

local function options_for_emotion(emotion, options)
    local preset = emotion_presets[emotion or "neutral"] or emotion_presets.neutral
    local merged = {}
    for key, value in pairs(preset) do
        merged[key] = value
    end
    for key, value in pairs(options or {}) do
        merged[key] = value
    end
    merged.emotion = emotion or merged.emotion or "neutral"
    return merged
end

function woman_voice.main(text)
    return woman_voice.speak(text or "BrainLLM is ready.")
end

function woman_voice.profile(emotion, options)
    return audio.profile("woman_default", options_for_emotion(emotion or "neutral", options))
end

function woman_voice.prompt(text)
    return audio.voice_prompt("woman_default", text or "BrainLLM is ready.")
end

function woman_voice.speak(text, emotion, options)
    return audio.speak(
        "woman_default",
        text or "BrainLLM is ready.",
        options_for_emotion(emotion or "neutral", options)
    )
end

function woman_voice.synthesize(text, output_path, emotion, options)
    return audio.synthesize(
        "woman_default",
        text or "BrainLLM is ready.",
        output_path or "assets/audio/woman/generated_response.wav",
        options_for_emotion(emotion or "neutral", options)
    )
end

function woman_voice.recognize(timeout_seconds, options)
    return audio.recognize(timeout_seconds or 6, options)
end

function woman_voice.listen(timeout_seconds)
    return woman_voice.recognize(timeout_seconds or 6, {
        expected_context = "user_prompt",
        language = "en-US"
    })
end

function woman_voice.reply(user_text, assistant_text, emotion)
    return audio.dialogue_turn(
        user_text or "",
        assistant_text or "I heard you.",
        options_for_emotion(emotion or "friendly")
    )
end

function woman_voice.listen_and_reply(timeout_seconds, fallback_text)
    return audio.sequence("woman_listen_and_reply", {
        woman_voice.listen(timeout_seconds or 6),
        woman_voice.speak(fallback_text or "I am listening.", "friendly")
    })
end

function woman_voice.startup_sequence()
    return audio.sequence("woman_startup_sequence", {
        woman_voice.startup_tone(),
        woman_voice.speak("BrainLLM is online.", "calm")
    })
end

function woman_voice.notification(event, text)
    return audio.notification(
        event or "general",
        text or "Notification received.",
        woman_voice.notify_tone()
    )
end

function woman_voice.error_message(text)
    return woman_voice.speak(text or "An error occurred.", "serious")
end

function woman_voice.status(text)
    return woman_voice.speak(text or "System status is nominal.", "calm")
end

function woman_voice.api_payload(action)
    return audio.api_payload(action or woman_voice.speak("BrainLLM is ready."))
end

function woman_voice.startup_tone()
    return audio.tone("woman_startup", 220.0, 750)
end

function woman_voice.notify_tone()
    return audio.tone("woman_notify", 440.0, 350)
end

return woman_voice
