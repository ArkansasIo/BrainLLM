#pragma once

#include "brain_types.h"
#include <string>
#include <vector>
#include <deque>

namespace BrainLLM {

// ========================================
// DIALOGUE AND CONVERSATION SYSTEM
// ========================================

struct ConversationTurn {
    uint64_t timestamp;
    std::string speaker;  // "user" or "ai"
    std::string message;
    std::string response;
    float confidence;
    std::string context;
};

struct DialogueContext {
    std::deque<ConversationTurn> history;
    std::string current_topic;
    std::vector<std::string> entities_mentioned;
    int turn_count;
    float conversation_coherence;
};

class DialogueManager {
public:
    DialogueManager(int history_depth = 10);
    ~DialogueManager() = default;
    
    // Dialogue operations
    std::string respond_to_user(const std::string& user_input);
    void add_turn(const ConversationTurn& turn);
    
    // Context management
    DialogueContext get_context() const;
    void update_context(const std::string& new_topic);
    
    // History
    std::vector<ConversationTurn> get_conversation_history() const;
    void clear_history();
    
    // Topic tracking
    std::string detect_topic_change(const std::string& input);
    float measure_coherence();
    
private:
    DialogueContext context_;
    int history_depth_;
    
    std::string generate_contextual_response(const std::string& input);
    void maintain_conversation_state(const std::string& input);
};

// ========================================
// PERSONALITY AND BEHAVIOR
// ========================================

struct PersonalityTraits {
    float openness;         // 0-1: Open to new ideas
    float conscientiousness; // 0-1: Organized, disciplined
    float extraversion;     // 0-1: Outgoing, social
    float agreeableness;    // 0-1: Cooperative, empathetic
    float neuroticism;      // 0-1: Emotional, reactive
};

struct BehaviorPattern {
    std::string pattern_name;
    std::vector<std::string> trigger_conditions;
    std::string response_template;
    float frequency;
    bool is_adaptive;
};

class PersonalityEngine {
public:
    PersonalityEngine();
    ~PersonalityEngine() = default;
    
    // Personality
    PersonalityTraits get_personality() const;
    void set_personality(const PersonalityTraits& traits);
    
    // Behavior
    std::string generate_response_with_personality(const std::string& input);
    std::vector<BehaviorPattern> get_behavior_patterns() const;
    
    // Adaptation
    void adapt_personality_from_feedback(const std::string& feedback);
    void learn_behavior_pattern(const BehaviorPattern& pattern);
    
private:
    PersonalityTraits personality_;
    std::vector<BehaviorPattern> behavior_patterns_;
    
    std::string apply_personality_filter(const std::string& text);
};

// ========================================
// EMOTION SIMULATION
// ========================================

enum class EmotionalState {
    Happy,
    Sad,
    Angry,
    Fearful,
    Surprised,
    Disgusted,
    Neutral,
    Confused
};

struct EmotionVector {
    float joy;
    float sadness;
    float anger;
    float fear;
    float surprise;
    float disgust;
    EmotionalState dominant_emotion;
};

class EmotionSimulator {
public:
    EmotionSimulator();
    ~EmotionSimulator() = default;
    
    // Emotion operations
    EmotionVector analyze_emotional_content(const std::string& text);
    EmotionalState get_current_emotional_state() const;
    
    // Emotion response
    void trigger_emotion(EmotionalState emotion, float intensity = 1.0f);
    std::string express_emotion(EmotionalState emotion);
    
    // Emotion dynamics
    void update_emotional_state();
    float get_emotion_intensity(EmotionalState emotion) const;
    
    // Learning emotions
    void learn_emotional_response(const std::string& stimulus, 
                                 EmotionalState response);
    
private:
    EmotionVector current_emotions_;
    std::map<std::string, EmotionalState> emotional_memory_;
    float emotion_decay_rate_;
};

} // namespace BrainLLM
