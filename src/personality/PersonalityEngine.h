#pragma once

#include "personality/PersonalityTypes.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace brainllm::personality {

struct PersonalityContext {
    std::string userMessage;
    std::string topic;
    double urgency{0.0};
    double emotionalIntensity{0.0};
    bool technicalTask{false};
};

struct PersonalityResponse {
    std::string tone;
    std::string instruction;
    double confidence{0.0};
};

class PersonalityEngine {
public:
    PersonalityEngine();

    void registerProfile(PersonalityProfile profile);
    bool setActiveProfile(const std::string& id);
    const PersonalityProfile* activeProfile() const;
    const PersonalityProfile* profile(const std::string& id) const;
    std::vector<std::string> profileIds() const;

    PersonalityResponse evaluate(const PersonalityContext& context) const;
    std::string buildSystemInstruction(const PersonalityContext& context) const;

private:
    std::unordered_map<std::string, PersonalityProfile> profiles_;
    std::string activeProfileId_;
};

} // namespace brainllm::personality
