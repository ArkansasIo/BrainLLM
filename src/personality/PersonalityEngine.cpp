#include "personality/PersonalityEngine.h"

#include <algorithm>
#include <sstream>

namespace brainllm::personality {

namespace {
PersonalityProfile makeProfile(const std::string& id, const std::string& name,
                               PersonalityKind kind, ResponseStyle style,
                               PersonalityTraits traits,
                               std::initializer_list<std::string> values) {
    PersonalityProfile p;
    p.id = id;
    p.displayName = name;
    p.kind = kind;
    p.responseStyle = style;
    p.traits = traits;
    p.values.assign(values.begin(), values.end());
    return p;
}
}

PersonalityEngine::PersonalityEngine() {
    registerProfile(makeProfile("balanced", "Balanced", PersonalityKind::Balanced,
        ResponseStyle::Conversational, {0.5,0.7,0.5,0.7,0.7,0.6,0.5,0.3,0.7},
        {"accuracy", "clarity", "helpfulness"}));

    registerProfile(makeProfile("analytical", "Analytical", PersonalityKind::Analytical,
        ResponseStyle::Technical, {0.7,0.9,0.3,0.5,0.8,0.8,0.7,0.1,0.4},
        {"evidence", "logic", "precision"}));

    registerProfile(makeProfile("creative", "Creative", PersonalityKind::Creative,
        ResponseStyle::Narrative, {0.95,0.6,0.7,0.7,0.6,0.95,0.4,0.8,0.7},
        {"originality", "experimentation", "storytelling"}));

    registerProfile(makeProfile("empathetic", "Empathetic", PersonalityKind::Empathetic,
        ResponseStyle::Conversational, {0.7,0.6,0.6,0.95,0.5,0.7,0.3,0.2,0.98},
        {"respect", "understanding", "support"}));

    registerProfile(makeProfile("commander", "Commander", PersonalityKind::Commander,
        ResponseStyle::Concise, {0.5,0.9,0.85,0.5,0.9,0.6,0.95,0.1,0.35},
        {"execution", "priorities", "decisiveness"}));

    registerProfile(makeProfile("mentor", "Mentor", PersonalityKind::Mentor,
        ResponseStyle::Socratic, {0.7,0.85,0.5,0.85,0.8,0.8,0.5,0.2,0.9},
        {"learning", "reasoning", "growth"}));

    registerProfile(makeProfile("stoic", "Stoic", PersonalityKind::Stoic,
        ResponseStyle::Concise, {0.45,0.9,0.25,0.5,0.95,0.5,0.65,0.05,0.3},
        {"discipline", "facts", "resilience"}));

    registerProfile(makeProfile("playful", "Playful", PersonalityKind::Playful,
        ResponseStyle::Conversational, {0.85,0.5,0.8,0.8,0.6,0.85,0.4,0.95,0.75},
        {"fun", "engagement", "curiosity"}));

    activeProfileId_ = "balanced";
}

void PersonalityEngine::registerProfile(PersonalityProfile profile) {
    if (!profile.id.empty()) profiles_[profile.id] = std::move(profile);
}

bool PersonalityEngine::setActiveProfile(const std::string& id) {
    if (!profiles_.contains(id)) return false;
    activeProfileId_ = id;
    return true;
}

const PersonalityProfile* PersonalityEngine::activeProfile() const {
    return profile(activeProfileId_);
}

const PersonalityProfile* PersonalityEngine::profile(const std::string& id) const {
    const auto it = profiles_.find(id);
    return it == profiles_.end() ? nullptr : &it->second;
}

std::vector<std::string> PersonalityEngine::profileIds() const {
    std::vector<std::string> result;
    result.reserve(profiles_.size());
    for (const auto& [id, _] : profiles_) result.push_back(id);
    std::sort(result.begin(), result.end());
    return result;
}

PersonalityResponse PersonalityEngine::evaluate(const PersonalityContext& context) const {
    const auto* p = activeProfile();
    if (!p) return {"neutral", "Respond clearly and accurately.", 0.0};

    PersonalityResponse result;
    result.confidence = 0.5 + (p->traits.conscientiousness * 0.4);
    if (context.technicalTask || p->responseStyle == ResponseStyle::Technical)
        result.tone = "precise, structured, technical";
    else if (context.emotionalIntensity > 0.7 && p->traits.empathy > 0.7)
        result.tone = "calm, supportive, empathetic";
    else if (p->traits.humor > 0.7)
        result.tone = "warm, playful, engaging";
    else
        result.tone = "clear, professional, conversational";

    std::ostringstream instruction;
    instruction << "Act as " << p->displayName << ". "
                << "Prioritize " << p->values.front() << ". "
                << "Use a " << result.tone << " tone. ";
    if (p->traits.assertiveness > 0.8)
        instruction << "Be decisive and state recommended actions directly. ";
    if (p->traits.curiosity > 0.8)
        instruction << "Explore alternatives and ask useful questions when information is missing. ";
    if (p->traits.empathy > 0.8)
        instruction << "Acknowledge user intent and emotional context without overdoing it. ";
    result.instruction = instruction.str();
    return result;
}

std::string PersonalityEngine::buildSystemInstruction(const PersonalityContext& context) const {
    return evaluate(context).instruction;
}

} // namespace brainllm::personality
