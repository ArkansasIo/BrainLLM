local common = require("scripts.lua.subscripts.common")

local bootstrap = {}

function bootstrap.main(context)
    context = context or {}
    return common.merge_context(context, {
        status = "bootstrapped",
        database_schema = "data/schema/brainllm_schema.sql",
        spreadsheet_layout = "data/spreadsheets/brainllm_layout.csv",
        voice_profile = "woman_default",
        speech_api = {
            status = "/api/speech/status",
            speak = "/api/speech/speak",
            synthesize = "/api/speech/synthesize",
            recognize = "/api/speech/recognize"
        }
    })
end

return bootstrap
