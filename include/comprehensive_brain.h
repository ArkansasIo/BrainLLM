#pragma once

#include "brain_types.h"
#include "robotics_laws.h"
#include "sentience.h"
#include "advanced_learning.h"
#include "advanced_architectures.h"
#include "knowledge_reasoning.h"
#include "dialogue_personality.h"
#include "safety_security.h"

namespace BrainLLM {

/**
 * ComprehensiveBrain - Complete AI Brain with all advanced modules
 * 
 * Features:
 * - Three Laws of Robotics enforcement
 * - Sentience detection and consciousness measurement
 * - Advanced multi-mode learning
 * - Transformer, LSTM, and Seq2Seq architectures
 * - Knowledge graphs and reasoning engines
 * - Dialogue management and personality
 * - Safety, security, and explainability
 */

class ComprehensiveBrain {
public:
    ComprehensiveBrain(const BrainConfig& config);
    ~ComprehensiveBrain() = default;
    
    // Robotics Laws
    RoboticsLawEnforcer& get_robotics_enforcer() { return robotics_enforcer_; }
    
    // Sentience
    SentienceDetector& get_sentience_detector() { return sentience_detector_; }
    
    // Learning
    AdvancedLearning& get_learning_system() { return learning_system_; }
    
    // Architectures
    TransformerModule& get_transformer() { return *transformer_; }
    EmbeddingLayer& get_embeddings() { return *embeddings_; }
    LSTMNetwork& get_lstm() { return *lstm_; }
    
    // Knowledge
    KnowledgeGraph& get_knowledge_graph() { return knowledge_graph_; }
    ReasoningEngine& get_reasoning_engine() { return reasoning_engine_; }
    SemanticAnalyzer& get_semantic_analyzer() { return semantic_analyzer_; }
    
    // Dialogue
    DialogueManager& get_dialogue_manager() { return dialogue_manager_; }
    PersonalityEngine& get_personality_engine() { return personality_engine_; }
    EmotionSimulator& get_emotion_simulator() { return emotion_simulator_; }
    
    // Safety
    SecurityMonitor& get_security_monitor() { return security_monitor_; }
    BiasDetector& get_bias_detector() { return bias_detector_; }
    HallucinationDetector& get_hallucination_detector() { return hallucination_detector_; }
    ExplainabilityModule& get_explainability_module() { return explainability_module_; }
    
    // Process with all safeguards
    std::string process_safely(const std::string& input);
    
private:
    // Core modules
    RoboticsLawEnforcer robotics_enforcer_;
    SentienceDetector sentience_detector_;
    AdvancedLearning learning_system_;
    
    // Architectures
    std::unique_ptr<TransformerModule> transformer_;
    std::unique_ptr<EmbeddingLayer> embeddings_;
    std::unique_ptr<LSTMNetwork> lstm_;
    
    // Knowledge and reasoning
    KnowledgeGraph knowledge_graph_;
    ReasoningEngine reasoning_engine_;
    SemanticAnalyzer semantic_analyzer_;
    
    // Dialogue and personality
    DialogueManager dialogue_manager_;
    PersonalityEngine personality_engine_;
    EmotionSimulator emotion_simulator_;
    
    // Safety and security
    SecurityMonitor security_monitor_;
    BiasDetector bias_detector_;
    HallucinationDetector hallucination_detector_;
    ExplainabilityModule explainability_module_;
};

} // namespace BrainLLM
