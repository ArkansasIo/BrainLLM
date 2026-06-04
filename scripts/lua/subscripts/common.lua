local common = {}

function common.log(message)
    return "[BrainLLM] " .. tostring(message)
end

function common.merge_context(base, extra)
    local merged = base or {}
    for key, value in pairs(extra or {}) do
        merged[key] = value
    end
    return merged
end

return common
