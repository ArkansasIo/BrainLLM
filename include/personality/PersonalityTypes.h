#pragma once

#include <string>
#include <vector>

namespace brainllm::personality {

enum class PersonalityKind {
    Balanced,
    Analytical,
    Creative,
    Empathetic,
    Assertive,
    Curious,
    Stoic,
    Playful,
    Mentor,
    Companion,
    Researcher,
    Commander
};

enum class ResponseStyle {
    Concise,
    Detailed,
    Technical,
    Conversational,
    Socratic,
    Narrative
};

struct PersonalityTraits {
    double openness{0.5};
    double conscientiousness{0.5};
    double extraversion{0.5};
    double agreeableness{0.5};
    double emotionalStability{0.5};
    double curiosity{0.5};
    double assertiveness{0.5};
    double humor{0.3};
    double empathy{0.5};
};

struct PersonalityProfile {
    std::string id;
    std::string displayName;
    std::string description;
    PersonalityKind kind{PersonalityKind::Balanced};
    ResponseStyle responseStyle{ResponseStyle::Conversational};
    PersonalityTraits traits{};
    std::vector<std::string> values;
    std::vector<std::string> preferredTopics;
    std::vector<std::string> avoidTopics;
};

} // namespace brainllm::personality
