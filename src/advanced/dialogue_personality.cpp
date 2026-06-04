#include "dialogue_personality.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <set>
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

uint64_t dialogue_now_seconds() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

std::vector<std::string> dialogue_words(const std::string& text) {
    std::vector<std::string> words;
    std::string current;
    for (unsigned char c : text) {
        if (std::isalnum(c)) {
            current += static_cast<char>(std::tolower(c));
        } else if (!current.empty()) {
            words.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) {
        words.push_back(current);
    }
    return words;
}

std::string topic_from_text(const std::string& text) {
    static const std::set<std::string> stop_words = {
        "the", "and", "for", "you", "that", "this", "with", "what", "how", "why",
        "are", "was", "were", "have", "has", "will", "can", "about"
    };
    for (const auto& word : dialogue_words(text)) {
        if (word.size() > 3 && stop_words.find(word) == stop_words.end()) {
            return word;
        }
    }
    return "general";
}

float overlap_ratio(const std::string& a, const std::string& b) {
    const auto left = dialogue_words(a);
    const auto right = dialogue_words(b);
    if (left.empty() || right.empty()) {
        return 0.0f;
    }
    std::set<std::string> left_set(left.begin(), left.end());
    std::set<std::string> right_set(right.begin(), right.end());
    int overlap = 0;
    for (const auto& word : left_set) {
        if (right_set.find(word) != right_set.end()) {
            ++overlap;
        }
    }
    return static_cast<float>(overlap) /
           static_cast<float>(std::max<size_t>(1, std::min(left_set.size(), right_set.size())));
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
    const std::string response = generate_contextual_response(user_input);
    ConversationTurn turn;
    turn.timestamp = dialogue_now_seconds();
    turn.speaker = "user";
    turn.message = user_input;
    turn.response = response;
    turn.confidence = context_.conversation_coherence;
    turn.context = context_.current_topic;
    add_turn(turn);
    return response;
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
    if (!new_topic.empty() && new_topic != "no_change") {
        context_.current_topic = new_topic;
    }
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
    const std::string topic = topic_from_text(input);
    if (context_.current_topic == "general") {
        return topic;
    }
    if (topic != "general" && topic != context_.current_topic &&
        overlap_ratio(topic, context_.current_topic) < 0.5f) {
        return topic;
    }
    return "no_change";
}

float DialogueManager::measure_coherence() {
    if (context_.history.empty()) {
        return context_.conversation_coherence;
    }
    const ConversationTurn& last = context_.history.back();
    context_.conversation_coherence =
        std::max(0.0f, std::min(1.0f, 0.65f + overlap_ratio(last.message, last.response) * 0.35f));
    return context_.conversation_coherence;
}

std::string DialogueManager::generate_contextual_response(const std::string& input) {
    std::ostringstream response;
    response << "Response";
    if (context_.turn_count > 0) {
        response << " in context of " << context_.current_topic;
    }
    response << ": I understand the request as focused on " << topic_from_text(input) << ".";
    if (!context_.history.empty()) {
        response << " Continuing from the previous turn, I will keep the answer coherent and actionable.";
    }
    return response.str();
}

void DialogueManager::maintain_conversation_state(const std::string& input) {
    const std::string topic_change = detect_topic_change(input);
    update_context(topic_change == "no_change" ? context_.current_topic : topic_change);
    for (const auto& word : dialogue_words(input)) {
        if (!word.empty() && std::isupper(static_cast<unsigned char>(word[0]))) {
            context_.entities_mentioned.push_back(word);
        }
    }
    measure_coherence();
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
    const std::string lower = normalize_key(feedback);
    if (lower.find("careful") != std::string::npos || lower.find("accurate") != std::string::npos) {
        personality_.conscientiousness = clamp_trait(personality_.conscientiousness + 0.05f);
    }
    if (lower.find("warm") != std::string::npos || lower.find("kind") != std::string::npos) {
        personality_.agreeableness = clamp_trait(personality_.agreeableness + 0.05f);
    }
    if (lower.find("creative") != std::string::npos) {
        personality_.openness = clamp_trait(personality_.openness + 0.05f);
    }
    if (lower.find("too") != std::string::npos && lower.find("much") != std::string::npos) {
        personality_.extraversion = clamp_trait(personality_.extraversion - 0.03f);
    }
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
    emotions.joy = 0.0f;
    emotions.sadness = 0.0f;
    emotions.anger = 0.0f;
    emotions.fear = 0.0f;
    emotions.surprise = 0.0f;
    emotions.disgust = 0.0f;
    const std::string lower = normalize_key(text);
    
    if (lower.find("happy") != std::string::npos || lower.find("great") != std::string::npos) {
        emotions.joy = 0.8f;
    }
    if (lower.find("sad") != std::string::npos || lower.find("hurt") != std::string::npos) {
        emotions.sadness = 0.8f;
    }
    if (lower.find("angry") != std::string::npos || lower.find("frustrated") != std::string::npos) {
        emotions.anger = 0.8f;
    }
    if (lower.find("afraid") != std::string::npos || lower.find("scared") != std::string::npos) {
        emotions.fear = 0.8f;
    }
    if (lower.find("wow") != std::string::npos || lower.find("surprised") != std::string::npos) {
        emotions.surprise = 0.7f;
    }

    float max_value = emotions.joy;
    emotions.dominant_emotion = EmotionalState::Happy;
    const std::vector<std::pair<float, EmotionalState>> candidates = {
        {emotions.sadness, EmotionalState::Sad},
        {emotions.anger, EmotionalState::Angry},
        {emotions.fear, EmotionalState::Fearful},
        {emotions.surprise, EmotionalState::Surprised},
        {emotions.disgust, EmotionalState::Disgusted}
    };
    for (const auto& candidate : candidates) {
        if (candidate.first > max_value) {
            max_value = candidate.first;
            emotions.dominant_emotion = candidate.second;
        }
    }
    if (max_value <= 0.0f) {
        emotions.dominant_emotion = EmotionalState::Neutral;
    }
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
        case EmotionalState::Fearful:
            current_emotions_.fear = intensity;
            break;
        case EmotionalState::Surprised:
            current_emotions_.surprise = intensity;
            break;
        case EmotionalState::Disgusted:
            current_emotions_.disgust = intensity;
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
        case EmotionalState::Fearful:
            return "I feel cautious.";
        case EmotionalState::Surprised:
            return "That surprised me.";
        case EmotionalState::Disgusted:
            return "I feel aversion toward that.";
        case EmotionalState::Confused:
            return "I feel uncertain and need more clarity.";
        default:
            return "I feel neutral.";
    }
}

void EmotionSimulator::update_emotional_state() {
    current_emotions_.joy *= emotion_decay_rate_;
    current_emotions_.sadness *= emotion_decay_rate_;
    current_emotions_.anger *= emotion_decay_rate_;
    current_emotions_.fear *= emotion_decay_rate_;
    current_emotions_.surprise *= emotion_decay_rate_;
    current_emotions_.disgust *= emotion_decay_rate_;
}

float EmotionSimulator::get_emotion_intensity(EmotionalState emotion) const {
    switch (emotion) {
        case EmotionalState::Happy:
            return current_emotions_.joy;
        case EmotionalState::Sad:
            return current_emotions_.sadness;
        case EmotionalState::Angry:
            return current_emotions_.anger;
        case EmotionalState::Fearful:
            return current_emotions_.fear;
        case EmotionalState::Surprised:
            return current_emotions_.surprise;
        case EmotionalState::Disgusted:
            return current_emotions_.disgust;
        default:
            return 0.0f;
    }
}

void EmotionSimulator::learn_emotional_response(const std::string& stimulus,
                                              EmotionalState response) {
    emotional_memory_[stimulus] = response;
}

} // namespace BrainLLM
