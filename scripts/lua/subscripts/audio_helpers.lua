local audio_helpers = {}

local function merge(base, override)
    local result = {}
    for key, value in pairs(base or {}) do
        result[key] = value
    end
    for key, value in pairs(override or {}) do
        result[key] = value
    end
    return result
end

local function clamp(value, min_value, max_value)
    value = tonumber(value) or min_value
    if value < min_value then return min_value end
    if value > max_value then return max_value end
    return value
end

function audio_helpers.profile(profile, options)
    options = options or {}
    return {
        profile = profile or "woman_default",
        language = options.language or "en-US",
        pitch_hz = clamp(options.pitch_hz or 220.0, 80.0, 420.0),
        speaking_rate = clamp(options.speaking_rate or 1.0, 0.5, 2.0),
        warmth = clamp(options.warmth or 0.75, 0.0, 1.0),
        emotion = options.emotion or "neutral",
        volume = clamp(options.volume or 100, 0, 100)
    }
end

function audio_helpers.voice_prompt(profile, text)
    return {
        action = "voice_prompt",
        profile = profile or "woman_default",
        text = text or "",
        output = "assets/audio/woman/generated_response.wav"
    }
end

function audio_helpers.speak(profile, text, options)
    local voice = audio_helpers.profile(profile, options)
    return {
        action = "speak",
        api = "/api/speech/speak",
        method = "POST",
        profile = voice.profile,
        voice = voice,
        text = text or ""
    }
end

function audio_helpers.synthesize(profile, text, output_path, options)
    local voice = audio_helpers.profile(profile, options)
    return {
        action = "synthesize",
        api = "/api/speech/synthesize",
        method = "POST",
        profile = voice.profile,
        voice = voice,
        text = text or "",
        output_path = output_path or "assets/audio/woman/generated_response.wav"
    }
end

function audio_helpers.recognize(timeout_seconds, options)
    options = options or {}
    return {
        action = "recognize",
        api = "/api/speech/recognize",
        method = "POST",
        timeout_seconds = timeout_seconds or 6,
        language = options.language or "en-US",
        expected_context = options.expected_context or "dictation"
    }
end

function audio_helpers.tone(asset_id, frequency_hz, duration_ms)
    return {
        action = "tone",
        asset_id = asset_id,
        frequency_hz = frequency_hz,
        duration_ms = duration_ms
    }
end

function audio_helpers.sequence(sequence_id, steps)
    return {
        action = "sequence",
        sequence_id = sequence_id or "voice_sequence",
        steps = steps or {}
    }
end

function audio_helpers.notification(event, text, tone)
    return {
        action = "notification",
        event = event or "general",
        tone = tone,
        speech = text
    }
end

function audio_helpers.dialogue_turn(user_text, assistant_text, options)
    return {
        action = "dialogue_turn",
        user_text = user_text or "",
        assistant_text = assistant_text or "",
        speak = audio_helpers.speak("woman_default", assistant_text or "", options)
    }
end

function audio_helpers.api_payload(action)
    action = action or {}
    return {
        endpoint = action.api or "",
        method = action.method or "POST",
        body = action
    }
end

return audio_helpers
