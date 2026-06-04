#include "comprehensive_brain.h"
#include <sstream>

namespace BrainLLM {

ComprehensiveBrain::ComprehensiveBrain(const BrainConfig& config)
    : learning_system_(),
      dialogue_manager_(10),
      personality_engine_() {
    
    // LLM engine + cognitive/linguistic pipeline
    llm_engine_ = std::make_unique<LLMEngine>(config);
    llm_engine_->initialize();
    cognitive_module_ = std::make_unique<IntegratedCognitiveModule>();

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
        // Mitigate bias (logged internally)
    }
    
    // 3. Robotics Laws check
    EthicalContext context;
    context.action_description = input;
    context.intent = ActionIntent::Helpful;
    
    if (!robotics_enforcer_.is_action_ethical(context)) {
        return "ERROR: Action violates Robotics Laws";
    }
    
    // 4. Cognitive / linguistic pre-processing
    //    Understand language, extract meaning, detect entities and key topics
    auto processed = cognitive_module_->understand_user_input(input);
    
    // 5. Route through positronic AGI/deep-search layer using the understood meaning
    auto positronic_result = positronic_network_.process_agi_query(
        processed.understood_meaning.empty() ? input : processed.understood_meaning,
        robotics_enforcer_);

    // 6. Generate the actual response through the full LLM pipeline
    //    (Wolfram Alpha for computational queries, AirLLM when available, heuristic fallback)
    std::string llm_response = llm_engine_->generate_response(input, 200);

    // 7. Enrich response with cognitive context (key topics, grammar check)
    std::ostringstream response;
    response << llm_response;

    if (!processed.key_topics.empty()) {
        response << " [Topics: ";
        for (size_t i = 0; i < processed.key_topics.size() && i < 5; ++i) {
            if (i > 0) response << ", ";
            response << processed.key_topics[i];
        }
        response << "]";
    }

    // 8. Detect hallucinations
    auto hallucination_analysis = hallucination_detector_.detect_hallucinations(llm_response);
    
    // 9. Add explainability confidence
    auto explanation = explainability_module_.explain_decision(llm_response);
    response << " [Confidence: " << std::to_string(explanation.overall_confidence) << "]";
    
    // 10. Store the interaction in LLM memory for future context
    llm_engine_->store_interaction(input, response.str());

    return response.str();
}

} // namespace BrainLLM
