local audio_helpers = {}

function audio_helpers.voice_prompt(profile, text)
    return {
        profile = profile or "woman_default",
        text = text or "",
        output = "assets/audio/woman/generated_response.wav"
    }
end

function audio_helpers.tone(asset_id, frequency_hz, duration_ms)
    return {
        asset_id = asset_id,
        frequency_hz = frequency_hz,
        duration_ms = duration_ms
    }
end

return audio_helpers
