#include "comprehensive_brain.h"

namespace BrainLLM {

ComprehensiveBrain::ComprehensiveBrain(const BrainConfig& config)
    : learning_system_(),
      dialogue_manager_(10),
      personality_engine_() {
    
    // Initialize architectures
    transformer_ = std::make_unique<TransformerModule>(
        config.num_layers,
        config.num_attention_heads,
        config.attention_dim
    );
    
    embeddings_ = std::make_unique<EmbeddingLayer>(
        config.vocab_size,
        config.embedding_dim
    );
    
    lstm_ = std::make_unique<LSTMNetwork>(
        config.num_layers,
        config.embedding_dim,
        config.attention_dim
    );
}

std::string ComprehensiveBrain::process_safely(const std::string& input) {
    // 1. Security check
    if (!security_monitor_.validate_input(input)) {
        return "ERROR: Input failed security validation";
    }
    
    // 2. Bias detection
    auto bias_analysis = bias_detector_.analyze_for_bias(input);
    if (bias_analysis.requires_mitigation) {
        // Mitigate bias
    }
    
    // 3. Robotics Laws check
    EthicalContext context;
    context.action_description = input;
    context.intent = ActionIntent::Helpful;
    
    if (!robotics_enforcer_.is_action_ethical(context)) {
        return "ERROR: Action violates Robotics Laws";
    }
    
    // 4. Process through neural architectures
    auto embedding = embeddings_->embed_token(static_cast<int>(input[0]));
    auto transformer_output = transformer_->forward(embedding);
    
    // 5. Detect hallucinations
    auto hallucination_analysis = hallucination_detector_.detect_hallucinations(input);
    
    // 6. Generate response
    std::string response = "Processed: " + input;
    
    // 7. Add explanation
    auto explanation = explainability_module_.explain_decision(response);
    response += " [Confidence: " + std::to_string(explanation.overall_confidence) + "]";
    
    return response;
}

} // namespace BrainLLM
