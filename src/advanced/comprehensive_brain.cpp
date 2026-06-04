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

    PositronicMemoryTrace safety_trace;
    safety_trace.id = "core-robotics-laws";
    safety_trace.content = "Prioritize human safety, truthful evidence, and grounded synthesis.";
    safety_trace.activation = 0.8f;
    safety_trace.emotional_salience = 0.4f;
    safety_trace.ethical_weight = 1.0f;
    safety_trace.timestamp = 0;
    positronic_network_.ingest_memory_trace(safety_trace);
}

std::string ComprehensiveBrain::process_safely(const std::string& input) {
    if (input.empty()) {
        return "ERROR: Empty input";
    }

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
    
    // 4. Route through positronic AGI/deep-search layer
    auto positronic_result = positronic_network_.process_agi_query(input, robotics_enforcer_);

    // 5. Process through neural architectures
    auto embedding = embeddings_->embed_token(static_cast<int>(input[0]));
    auto transformer_output = transformer_->forward(embedding);
    
    // 6. Detect hallucinations
    auto hallucination_analysis = hallucination_detector_.detect_hallucinations(input);
    
    // 7. Generate response
    std::string response = "Processed: " + input + " [" + positronic_result + "]";
    
    // 8. Add explanation
    auto explanation = explainability_module_.explain_decision(response);
    response += " [Confidence: " + std::to_string(explanation.overall_confidence) + "]";
    
    return response;
}

} // namespace BrainLLM
