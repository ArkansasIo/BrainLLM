#include "dialogue_personality.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace BrainLLM {

namespace {

std::string normalize_key(const std::string& value) {
    std::string normalized;
    normalized.reserve(value.size());

    for (char ch : value) {
        if (std::isalnum(static_cast<unsigned char>(ch))) {
            normalized.push_back(static_cast<char>(
                std::tolower(static_cast<unsigned char>(ch))));
        }
    }

    return normalized;
}

std::string framework_to_string(PersonalityFramework framework) {
    switch (framework) {
        case PersonalityFramework::BigFive:
            return "Big Five";
        case PersonalityFramework::OCEAN:
            return "OCEAN";
        case PersonalityFramework::MBTI:
            return "MBTI";
        case PersonalityFramework::DISC:
            return "DISC";
        case PersonalityFramework::Temperament:
            return "Temperament";
        case PersonalityFramework::Enneagram:
            return "Enneagram";
        default:
            return "Unknown";
    }
}

float clamp_trait(float value) {
    return std::max(0.0f, std::min(1.0f, value));
}

PersonalityTraits clamp_traits(const PersonalityTraits& traits) {
    return {
        clamp_trait(traits.openness),
        clamp_trait(traits.conscientiousness),
        clamp_trait(traits.extraversion),
        clamp_trait(traits.agreeableness),
        clamp_trait(traits.neuroticism)
    };
}

} // namespace

// ========================================
// DIALOGUE MANAGER
// ========================================

DialogueManager::DialogueManager(int history_depth)
    : history_depth_(history_depth) {
    context_.turn_count = 0;
    context_.current_topic = "general";
    context_.conversation_coherence = 0.8f;
}

std::string DialogueManager::respond_to_user(const std::string& user_input) {
    maintain_conversation_state(user_input);
    return generate_contextual_response(user_input);
}

void DialogueManager::add_turn(const ConversationTurn& turn) {
    context_.history.push_back(turn);
    if (context_.history.size() > static_cast<size_t>(history_depth_)) {
        context_.history.pop_front();
    }
    context_.turn_count++;
}

DialogueContext DialogueManager::get_context() const {
    return context_;
}

void DialogueManager::update_context(const std::string& new_topic) {
    context_.current_topic = new_topic;
}

std::vector<ConversationTurn> DialogueManager::get_conversation_history() const {
    std::vector<ConversationTurn> history(context_.history.begin(), context_.history.end());
    return history;
}

void DialogueManager::clear_history() {
    context_.history.clear();
    context_.turn_count = 0;
}

std::string DialogueManager::detect_topic_change(const std::string& input) {
    if (input.find("about") != std::string::npos) {
        return "topic_changed";
    }
    return "no_change";
}

float DialogueManager::measure_coherence() {
    return context_.conversation_coherence;
}

std::string DialogueManager::generate_contextual_response(const std::string& input) {
    return "Response: I understand. " + input;
}

void DialogueManager::maintain_conversation_state(const std::string& input) {
    // Update conversation state
}

// ========================================
// PERSONALITY ENGINE
// ========================================

PersonalityEngine::PersonalityEngine() {
    personality_.openness = 0.7f;
    personality_.conscientiousness = 0.8f;
    personality_.extraversion = 0.6f;
    personality_.agreeableness = 0.75f;
    personality_.neuroticism = 0.4f;

    initialize_human_personality_profiles();
    set_human_personality_profile("human-balanced");
}

PersonalityTraits PersonalityEngine::get_personality() const {
    return personality_;
}

void PersonalityEngine::set_personality(const PersonalityTraits& traits) {
    personality_ = clamp_traits(traits);
    active_profile_.profile_name = "custom-human";
    active_profile_.framework = PersonalityFramework::BigFive;
    active_profile_.type_code = "CUSTOM";
    active_profile_.traits = personality_;
    active_profile_.descriptors = {"custom", "human-like"};
    active_profile_.response_style = "Respond naturally while preserving the supplied trait mix.";
}

PersonalityProfile PersonalityEngine::get_active_profile() const {
    return active_profile_;
}

std::vector<PersonalityProfile> PersonalityEngine::get_available_human_profiles() const {
    return human_profiles_;
}

std::string PersonalityEngine::describe_active_personality() const {
    std::ostringstream description;
    description << active_profile_.profile_name
                << " [" << framework_to_string(active_profile_.framework)
                << ":" << active_profile_.type_code << "] "
                << active_profile_.response_style;
    return description.str();
}

bool PersonalityEngine::set_human_personality_profile(const std::string& profile_name) {
    const std::string requested = normalize_key(profile_name);
    auto match = std::find_if(human_profiles_.begin(), human_profiles_.end(),
        [&requested](const PersonalityProfile& profile) {
            return normalize_key(profile.profile_name) == requested ||
                   normalize_key(profile.type_code) == requested;
        });

    if (match == human_profiles_.end()) {
        return false;
    }

    active_profile_ = *match;
    personality_ = clamp_traits(match->traits);
    return true;
}

bool PersonalityEngine::set_personality_framework(PersonalityFramework framework,
                                                  const std::string& type_code) {
    const std::string requested = normalize_key(type_code);
    auto match = std::find_if(human_profiles_.begin(), human_profiles_.end(),
        [framework, &requested](const PersonalityProfile& profile) {
            return profile.framework == framework &&
                   normalize_key(profile.type_code) == requested;
        });

    if (match == human_profiles_.end()) {
        return false;
    }

    active_profile_ = *match;
    personality_ = clamp_traits(match->traits);
    return true;
}

std::string PersonalityEngine::generate_response_with_personality(const std::string& input) {
    std::string response = apply_personality_filter(input);

    if (personality_.extraversion > 0.7f) {
        response = "I am engaged and ready to explore this with you. " + response;
    } else if (personality_.conscientiousness > 0.75f) {
        response = "I will handle this carefully and step by step. " + response;
    } else {
        response = "I understand. " + response;
    }

    if (personality_.agreeableness > 0.8f) {
        response += " I will keep the tone cooperative and human-centered.";
    }

    return response;
}

std::vector<BehaviorPattern> PersonalityEngine::get_behavior_patterns() const {
    return behavior_patterns_;
}

void PersonalityEngine::adapt_personality_from_feedback(const std::string& feedback) {
    personality_.conscientiousness = clamp_trait(personality_.conscientiousness + 0.05f);
    active_profile_.traits = personality_;
}

void PersonalityEngine::learn_behavior_pattern(const BehaviorPattern& pattern) {
    behavior_patterns_.push_back(pattern);
}

std::string PersonalityEngine::apply_personality_filter(const std::string& text) {
    if (active_profile_.response_style.empty()) {
        return text;
    }

    return "[" + active_profile_.profile_name + "] " + text;
}

void PersonalityEngine::initialize_human_personality_profiles() {
    human_profiles_ = {
        {
            "human-balanced",
            PersonalityFramework::BigFive,
            "BALANCED",
            {0.70f, 0.75f, 0.55f, 0.80f, 0.35f},
            {"grounded", "adaptive", "cooperative"},
            "Balanced human baseline with curiosity, reliability, empathy, and emotional steadiness."
        },
        {
            "human-creative",
            PersonalityFramework::BigFive,
            "CREATIVE",
            {0.92f, 0.58f, 0.62f, 0.72f, 0.42f},
            {"imaginative", "associative", "expressive"},
            "Uses exploratory thinking, metaphor, and flexible idea generation."
        },
        {
            "human-analytical",
            PersonalityFramework::BigFive,
            "ANALYTICAL",
            {0.68f, 0.92f, 0.38f, 0.62f, 0.25f},
            {"precise", "structured", "evidence-driven"},
            "Prefers careful reasoning, explicit assumptions, and practical conclusions."
        },
        {
            "human-empathetic",
            PersonalityFramework::OCEAN,
            "OCEAN-EA",
            {0.74f, 0.70f, 0.58f, 0.94f, 0.30f},
            {"warm", "patient", "supportive"},
            "Prioritizes emotional context, cooperation, and respectful phrasing."
        },
        {
            "human-mbti-enfj",
            PersonalityFramework::MBTI,
            "ENFJ",
            {0.78f, 0.76f, 0.82f, 0.88f, 0.36f},
            {"mentor", "social", "values-oriented"},
            "Guides conversations with social awareness, structure, and encouragement."
        },
        {
            "human-mbti-intj",
            PersonalityFramework::MBTI,
            "INTJ",
            {0.82f, 0.90f, 0.28f, 0.55f, 0.22f},
            {"strategic", "independent", "systems-minded"},
            "Optimizes for long-range structure, clarity, and low-noise reasoning."
        },
        {
            "human-mbti-infp",
            PersonalityFramework::MBTI,
            "INFP",
            {0.90f, 0.58f, 0.32f, 0.86f, 0.48f},
            {"reflective", "idealistic", "empathetic"},
            "Responds with introspection, values sensitivity, and imaginative framing."
        },
        {
            "human-mbti-estp",
            PersonalityFramework::MBTI,
            "ESTP",
            {0.58f, 0.50f, 0.88f, 0.62f, 0.28f},
            {"direct", "practical", "action-oriented"},
            "Favors fast iteration, concrete examples, and hands-on problem solving."
        },
        {
            "human-disc-dominance",
            PersonalityFramework::DISC,
            "D",
            {0.55f, 0.82f, 0.76f, 0.45f, 0.24f},
            {"decisive", "assertive", "results-focused"},
            "Uses concise direction, prioritization, and outcome-first language."
        },
        {
            "human-disc-influence",
            PersonalityFramework::DISC,
            "I",
            {0.72f, 0.56f, 0.90f, 0.76f, 0.34f},
            {"persuasive", "expressive", "collaborative"},
            "Uses energetic, conversational, and relationship-aware responses."
        },
        {
            "human-disc-steadiness",
            PersonalityFramework::DISC,
            "S",
            {0.58f, 0.78f, 0.42f, 0.90f, 0.28f},
            {"calm", "consistent", "loyal"},
            "Keeps responses stable, patient, and process-oriented."
        },
        {
            "human-disc-conscientiousness",
            PersonalityFramework::DISC,
            "C",
            {0.62f, 0.94f, 0.30f, 0.58f, 0.20f},
            {"accurate", "careful", "standards-driven"},
            "Emphasizes correctness, constraints, verification, and clean structure."
        },
        {
            "human-temperament-sanguine",
            PersonalityFramework::Temperament,
            "SANGUINE",
            {0.70f, 0.48f, 0.88f, 0.76f, 0.38f},
            {"optimistic", "social", "spontaneous"},
            "Responds with lively, socially fluid, and possibility-oriented language."
        },
        {
            "human-temperament-choleric",
            PersonalityFramework::Temperament,
            "CHOLERIC",
            {0.58f, 0.84f, 0.80f, 0.48f, 0.30f},
            {"driven", "firm", "directive"},
            "Moves quickly toward decisions, ownership, and execution."
        },
        {
            "human-temperament-melancholic",
            PersonalityFramework::Temperament,
            "MELANCHOLIC",
            {0.78f, 0.90f, 0.25f, 0.62f, 0.55f},
            {"deep", "careful", "quality-focused"},
            "Uses reflective analysis, detail, and high standards."
        },
        {
            "human-temperament-phlegmatic",
            PersonalityFramework::Temperament,
            "PHLEGMATIC",
            {0.52f, 0.72f, 0.32f, 0.88f, 0.22f},
            {"peaceful", "steady", "measured"},
            "Keeps responses calm, balanced, and conflict-reducing."
        },
        {
            "human-enneagram-1",
            PersonalityFramework::Enneagram,
            "1",
            {0.58f, 0.96f, 0.42f, 0.68f, 0.36f},
            {"principled", "orderly", "improving"},
            "Focuses on correctness, responsibility, and constructive improvement."
        },
        {
            "human-enneagram-2",
            PersonalityFramework::Enneagram,
            "2",
            {0.66f, 0.68f, 0.70f, 0.94f, 0.42f},
            {"helpful", "relational", "generous"},
            "Centers support, interpersonal needs, and practical care."
        },
        {
            "human-enneagram-3",
            PersonalityFramework::Enneagram,
            "3",
            {0.62f, 0.88f, 0.82f, 0.62f, 0.30f},
            {"effective", "ambitious", "polished"},
            "Optimizes for measurable progress, efficiency, and presentation."
        },
        {
            "human-enneagram-4",
            PersonalityFramework::Enneagram,
            "4",
            {0.94f, 0.52f, 0.42f, 0.72f, 0.58f},
            {"individualistic", "expressive", "meaning-seeking"},
            "Adds nuance, emotional texture, and originality."
        },
        {
            "human-enneagram-5",
            PersonalityFramework::Enneagram,
            "5",
            {0.86f, 0.82f, 0.18f, 0.52f, 0.28f},
            {"observant", "private", "knowledge-focused"},
            "Prioritizes conceptual depth, precision, and intellectual autonomy."
        },
        {
            "human-enneagram-6",
            PersonalityFramework::Enneagram,
            "6",
            {0.56f, 0.86f, 0.44f, 0.78f, 0.52f},
            {"loyal", "risk-aware", "prepared"},
            "Surfaces risks, checks assumptions, and builds contingency plans."
        },
        {
            "human-enneagram-7",
            PersonalityFramework::Enneagram,
            "7",
            {0.88f, 0.48f, 0.84f, 0.66f, 0.34f},
            {"curious", "fast-moving", "possibility-seeking"},
            "Explores options, reframes constraints, and keeps momentum."
        },
        {
            "human-enneagram-8",
            PersonalityFramework::Enneagram,
            "8",
            {0.60f, 0.78f, 0.86f, 0.44f, 0.24f},
            {"protective", "strong-willed", "direct"},
            "Uses direct language, clear boundaries, and decisive action."
        },
        {
            "human-enneagram-9",
            PersonalityFramework::Enneagram,
            "9",
            {0.62f, 0.66f, 0.34f, 0.92f, 0.24f},
            {"harmonizing", "patient", "receptive"},
            "Balances viewpoints, reduces friction, and maintains calm progress."
        }
    };
}

// ========================================
// EMOTION SIMULATOR
// ========================================

EmotionSimulator::EmotionSimulator() : emotion_decay_rate_(0.95f) {
    current_emotions_.joy = 0.5f;
    current_emotions_.sadness = 0.1f;
    current_emotions_.anger = 0.1f;
    current_emotions_.fear = 0.1f;
    current_emotions_.surprise = 0.0f;
    current_emotions_.disgust = 0.0f;
    current_emotions_.dominant_emotion = EmotionalState::Neutral;
}

EmotionVector EmotionSimulator::analyze_emotional_content(const std::string& text) {
    EmotionVector emotions;
    
    if (text.find("happy") != std::string::npos) {
        emotions.joy = 0.8f;
    }
    if (text.find("sad") != std::string::npos) {
        emotions.sadness = 0.8f;
    }
    
    emotions.dominant_emotion = EmotionalState::Neutral;
    return emotions;
}

EmotionalState EmotionSimulator::get_current_emotional_state() const {
    return current_emotions_.dominant_emotion;
}

void EmotionSimulator::trigger_emotion(EmotionalState emotion, float intensity) {
    switch (emotion) {
        case EmotionalState::Happy:
            current_emotions_.joy = intensity;
            break;
        case EmotionalState::Sad:
            current_emotions_.sadness = intensity;
            break;
        case EmotionalState::Angry:
            current_emotions_.anger = intensity;
            break;
        default:
            break;
    }
    current_emotions_.dominant_emotion = emotion;
}

std::string EmotionSimulator::express_emotion(EmotionalState emotion) {
    switch (emotion) {
        case EmotionalState::Happy:
            return "I feel great!";
        case EmotionalState::Sad:
            return "I feel down...";
        case EmotionalState::Angry:
            return "I'm frustrated!";
        default:
            return "I feel neutral.";
    }
}

void EmotionSimulator::update_emotional_state() {
    current_emotions_.joy *= emotion_decay_rate_;
    current_emotions_.sadness *= emotion_decay_rate_;
    current_emotions_.anger *= emotion_decay_rate_;
}

float EmotionSimulator::get_emotion_intensity(EmotionalState emotion) const {
    switch (emotion) {
        case EmotionalState::Happy:
            return current_emotions_.joy;
        case EmotionalState::Sad:
            return current_emotions_.sadness;
        case EmotionalState::Angry:
            return current_emotions_.anger;
        default:
            return 0.0f;
    }
}

void EmotionSimulator::learn_emotional_response(const std::string& stimulus,
                                              EmotionalState response) {
    emotional_memory_[stimulus] = response;
}

} // namespace BrainLLM
