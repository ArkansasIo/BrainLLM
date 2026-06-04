local common = require("scripts.lua.subscripts.common")

local bootstrap = {}

function bootstrap.main(context)
    context = context or {}
    return common.merge_context(context, {
        status = "bootstrapped",
        database_schema = "data/schema/brainllm_schema.sql",
        spreadsheet_layout = "data/spreadsheets/brainllm_layout.csv",
        voice_profile = "woman_default"
    })
end

return bootstrap
