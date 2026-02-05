#ifndef QUANTUM_LLM_INTEGRATION_H
#define QUANTUM_LLM_INTEGRATION_H

#include "quantum_computing.h"
#include "integrated_cognitive_module.h"

#include <vector>
#include <string>
#include <memory>
#include <map>

namespace BrainLLM {

/**
 * Quantum-Enhanced LLM
 * 
 * Integrates quantum computing with classical language models for:
 * - Superposition-based text encoding
 * - Quantum semantic search
 * - Quantum-enhanced language translation
 * - Quantum feature extraction
 */

class QuantumLLMIntegration {
public:
    QuantumLLMIntegration(int num_qubits = 10);
    
    // ========== QUANTUM TEXT ENCODING ==========
    
    /**
     * Encode text into quantum superposition
     * Multiple word embeddings in superposition for parallel processing
     */
    struct QuantumTextEncoding {
        QuantumRegister quantum_state;
        std::vector<float> amplitude_map;      // Maps amplitudes to word indices
        std::vector<std::string> encoded_tokens;
        float entanglement_degree;
    };
    
    QuantumTextEncoding encode_text_quantum(const std::string& text);
    
    // ========== QUANTUM SEMANTIC SEARCH ==========
    
    /**
     * Use Grover's algorithm for semantic search
     * Quadratic speedup over classical search
     */
    struct QuantumSearchResult {
        std::vector<std::string> matches;
        std::vector<float> relevance_scores;
        int iterations_used;
        float classical_equivalent_time_ms;
        float quantum_time_ms;
    };
    
    QuantumSearchResult quantum_semantic_search(
        const std::string& query,
        const std::vector<std::string>& documents
    );
    
    // ========== QUANTUM LANGUAGE TRANSLATION ==========
    
    /**
     * Enhanced translation using quantum superposition
     * Process multiple translation hypotheses simultaneously
     */
    struct QuantumTranslationResult {
        std::string translated_text;
        std::vector<std::string> alternative_translations;
        std::vector<float> confidence_per_alternative;
        float quantum_quality_boost;
    };
    
    QuantumTranslationResult quantum_translate(
        const std::string& text,
        const std::string& source_language,
        const std::string& target_language
    );
    
    // ========== QUANTUM FEATURE EXTRACTION ==========
    
    /**
     * Extract features using quantum feature maps
     * Encode classical data into high-dimensional quantum space
     */
    struct QuantumFeatures {
        std::vector<float> classical_features;
        std::vector<std::complex<float>> quantum_features;
        int dimensionality_increase;
        float quantum_advantage_ratio;
    };
    
    QuantumFeatures extract_quantum_features(
        const std::vector<float>& classical_features
    );
    
    // ========== QUANTUM SENTIMENT ANALYSIS ==========
    
    /**
     * Enhanced sentiment analysis with quantum amplitude encoding
     * Process multiple sentiment dimensions in superposition
     */
    struct QuantumSentimentAnalysis {
        float primary_sentiment;           // -1 to +1
        std::vector<float> sentiment_spectrum;  // Fine-grained spectrum
        std::map<std::string, float> emotion_amplitudes;
        float quantum_confidence;
    };
    
    QuantumSentimentAnalysis analyze_sentiment_quantum(const std::string& text);
    
    // ========== QUANTUM ENTITY EXTRACTION ==========
    
    /**
     * Extract entities using quantum search
     * Identify all entities simultaneously
     */
    struct QuantumEntityExtraction {
        std::vector<std::string> entities;
        std::vector<std::string> entity_types;
        std::vector<float> confidence_scores;
        int speedup_iterations;
    };
    
    QuantumEntityExtraction extract_entities_quantum(const std::string& text);
    
    // ========== QUANTUM GRAMMAR ANALYSIS ==========
    
    /**
     * Grammar checking enhanced with quantum algorithms
     * Check multiple grammar rules in parallel
     */
    struct QuantumGrammarAnalysis {
        bool is_grammatical;
        std::vector<std::string> errors_detected;
        std::vector<std::string> corrections;
        std::vector<float> error_probabilities;
        float quantum_analysis_depth;
    };
    
    QuantumGrammarAnalysis check_grammar_quantum(const std::string& text);
    
    // ========== QUANTUM LANGUAGE UNDERSTANDING ==========
    
    /**
     * Unified quantum-enhanced language understanding
     * Combines all quantum features for comprehensive analysis
     */
    struct QuantumLanguageUnderstanding {
        std::string primary_meaning;
        std::vector<std::string> alternative_interpretations;
        std::vector<float> interpretation_amplitudes;
        QuantumSentimentAnalysis sentiment;
        QuantumEntityExtraction entities;
        QuantumGrammarAnalysis grammar;
        float overall_quantum_advantage;
    };
    
    QuantumLanguageUnderstanding understand_text_quantum(const std::string& text);
    
    // ========== QUANTUM RESPONSE GENERATION ==========
    
    /**
     * Generate responses considering multiple possibilities in superposition
     */
    struct QuantumResponse {
        std::string primary_response;
        std::vector<std::string> alternative_responses;
        std::vector<float> response_probabilities;
        float relevance_score;
        int quantum_generation_depth;
    };
    
    QuantumResponse generate_response_quantum(
        const std::string& prompt,
        const std::string& language = "en"
    );
    
    // ========== QUANTUM LEARNING ENHANCEMENT ==========
    
    /**
     * Enhance classical learning with quantum advantage
     */
    struct QuantumLearningBoost {
        std::vector<float> enhanced_features;
        float feature_space_expansion;
        int complexity_reduction;
        float learning_acceleration;
    };
    
    QuantumLearningBoost enhance_learning_quantum(
        const std::vector<float>& classical_data
    );
    
    // ========== INTEGRATION WITH COGNITIVE MODULE ==========
    
    /**
     * Hybrid system combining classical cognition and quantum processing
     */
    struct HybridCognitiveQuantum {
        IntegratedCognitiveModule::ProcessedInput cognitive_analysis;
        QuantumLanguageUnderstanding quantum_analysis;
        QuantumResponse quantum_response;
        float hybrid_confidence;
        float processing_time_reduction;
    };
    
    HybridCognitiveQuantum process_hybrid(const std::string& user_input);
    
    // ========== UTILITY METHODS ==========
    
    // Get current quantum state information
    struct QuantumStateInfo {
        int num_qubits;
        int entanglement_degree;
        float decoherence_rate;
        std::vector<float> qubit_amplitudes;
    };
    
    QuantumStateInfo get_quantum_state_info() const;
    
    // Reset quantum processor
    void reset_quantum_state();
    
    // Get quantum advantage metrics
    struct QuantumAdvantageMetrics {
        float average_speedup;
        float accuracy_improvement;
        float feature_dimensionality_increase;
        int parallel_computations;
    };
    
    QuantumAdvantageMetrics get_advantage_metrics() const;
    
    // Enable/disable quantum processing
    void set_quantum_enabled(bool enabled) { quantum_enabled_ = enabled; }
    bool is_quantum_enabled() const { return quantum_enabled_; }
    
private:
    int num_qubits_;
    bool quantum_enabled_ = true;
    
    std::unique_ptr<QuantumRegister> quantum_register_;
    std::unique_ptr<QuantumCircuit> quantum_circuit_;
    std::unique_ptr<GroversAlgorithm> grovers_algorithm_;
    std::unique_ptr<QuantumFourierTransform> qft_;
    std::unique_ptr<QuantumSVM> quantum_svm_;
    std::unique_ptr<HybridQuantumClassical> hybrid_processor_;
    std::unique_ptr<QuantumStateTomography> state_tomography_;
    
    // Helper methods
    std::vector<float> encode_classical_to_quantum(const std::vector<float>& data);
    std::vector<float> decode_quantum_to_classical(
        const std::vector<std::complex<float>>& quantum_data
    );
    
    float compute_quantum_advantage(int classical_complexity, 
                                   int quantum_complexity);
};

/**
 * Quantum Language Model
 * Full LLM enhanced with quantum capabilities
 */
class QuantumLanguageModel {
public:
    QuantumLanguageModel(int num_qubits = 15);
    
    // Process text with quantum enhancement
    std::string process_text(const std::string& input);
    
    // Train quantum parameters
    void train_quantum_parameters(const std::vector<std::string>& training_data);
    
    // Get model statistics
    struct ModelStats {
        int num_parameters;
        float quantum_accuracy;
        float classical_accuracy;
        float speedup_factor;
        float entanglement_entropy;
    };
    
    ModelStats get_statistics() const;
    
private:
    std::unique_ptr<QuantumLLMIntegration> quantum_integration_;
    std::unique_ptr<IntegratedCognitiveModule> classical_module_;
    std::vector<float> quantum_parameters_;
};

} // namespace BrainLLM

#endif // QUANTUM_LLM_INTEGRATION_H
