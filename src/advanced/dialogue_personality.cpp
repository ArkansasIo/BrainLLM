#include "dialogue_personality.h"

namespace BrainLLM {

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
}

PersonalityTraits PersonalityEngine::get_personality() const {
    return personality_;
}

void PersonalityEngine::set_personality(const PersonalityTraits& traits) {
    personality_ = traits;
}

std::string PersonalityEngine::generate_response_with_personality(const std::string& input) {
    std::string response = input;
    
    if (personality_.extraversion > 0.6f) {
        response = "Hey! " + response + " Let's talk more!";
    } else {
        response = "I see. " + response;
    }
    
    return response;
}

std::vector<BehaviorPattern> PersonalityEngine::get_behavior_patterns() const {
    return behavior_patterns_;
}

void PersonalityEngine::adapt_personality_from_feedback(const std::string& feedback) {
    personality_.conscientiousness += 0.05f;
}

void PersonalityEngine::learn_behavior_pattern(const BehaviorPattern& pattern) {
    behavior_patterns_.push_back(pattern);
}

std::string PersonalityEngine::apply_personality_filter(const std::string& text) {
    return text;
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
