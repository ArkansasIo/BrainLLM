local audio = require("scripts.lua.subscripts.audio_helpers")

local woman_voice = {}

function woman_voice.main(text)
    return audio.voice_prompt("woman_default", text or "BrainLLM is ready.")
end

function woman_voice.startup_tone()
    return audio.tone("woman_startup", 220.0, 750)
end

function woman_voice.notify_tone()
    return audio.tone("woman_notify", 440.0, 350)
end

return woman_voice
