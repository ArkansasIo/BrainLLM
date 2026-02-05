#ifndef INTEGRATED_COGNITIVE_MODULE_H
#define INTEGRATED_COGNITIVE_MODULE_H

#include "cognitive_processes.h"
#include "english_processor.h"
#include "multilingual_processor.h"
#include "grammar_linguistics.h"

#include <memory>
#include <string>
#include <vector>
#include <map>

namespace BrainLLM {

/**
 * IntegratedCognitiveModule
 * 
 * High-level integration of all cognitive and linguistic modules.
 * Provides unified interface for human-like thinking with multilingual support.
 * Combines:
 *  - Cognitive processes (attention, memory, decision-making)
 *  - English language processing
 *  - Multilingual translation and analysis
 *  - Grammar and linguistic analysis
 */
class IntegratedCognitiveModule {
public:
    IntegratedCognitiveModule();
    ~IntegratedCognitiveModule();
    
    // ========== COGNITIVE THINKING ==========
    
    /// Add information to working memory
    void add_thought(const std::string& thought);
    
    /// Focus cognitive attention on topic
    void focus_attention(const std::string& topic, float intensity = 0.8f);
    
    /// Make a decision with cognitive reasoning
    std::string think_and_decide(const std::vector<std::string>& options,
                                const std::map<std::string, float>& option_values);
    
    /// Retrieve related memories
    std::vector<std::string> recall_related_thoughts(const std::string& cue);
    
    /// Get current cognitive state
    struct CognitiveState {
        int working_memory_load;
        float cognitive_fatigue;
        std::string attention_focus;
        float attention_intensity;
        bool is_overloaded;
    };
    
    CognitiveState get_cognitive_state() const;
    
    // ========== ENGLISH LANGUAGE PROCESSING ==========
    
    /// Analyze English text completely
    struct EnglishAnalysis {
        std::vector<std::string> sentences;
        std::vector<EnglishProcessor::WordAnalysis> pos_tags;
        float sentiment;
        float readability;
        std::string style;
        std::vector<std::string> entities;
        std::string main_idea;
    };
    
    EnglishAnalysis analyze_english_text(const std::string& text);
    
    /// Process English sentence for grammar
    bool check_english_grammar(const std::string& sentence);
    
    /// Generate grammatically correct English
    std::string generate_english_sentence(const std::vector<std::string>& words);
    
    // ========== MULTILINGUAL SUPPORT ==========
    
    /// Detect language and analyze
    struct LanguageAnalysis {
        MultiLanguageProcessor::Language language;
        float detection_confidence;
        MultiLanguageProcessor::LanguageFeatures features;
        std::vector<std::string> normalized_tokens;
    };
    
    LanguageAnalysis analyze_multilingual_text(const std::string& text);
    
    /// Translate between languages
    struct TranslationResult {
        std::string translated_text;
        MultiLanguageProcessor::Language source_language;
        MultiLanguageProcessor::Language target_language;
        float translation_confidence;
        std::vector<std::string> alternatives;
    };
    
    TranslationResult translate_text(const std::string& text,
                                    const std::string& target_language);
    
    /// Get language metadata
    MultiLanguageProcessor::LanguageMetadata get_language_info(const std::string& language_code);
    
    // ========== ADVANCED GRAMMAR & LINGUISTICS ==========
    
    /// Complete linguistic analysis
    struct LinguisticAnalysis {
        std::vector<GrammarError> grammar_errors;
        float grammar_score;
        LinguisticAnalyzer::StyleAnalysis style;
        LinguisticAnalyzer::Stylometry stylometry;
        LinguisticAnalyzer::RhetoricAnalysis rhetoric;
        std::vector<LinguisticFeature> features;
    };
    
    LinguisticAnalysis full_linguistic_analysis(const std::string& text);
    
    /// Check and correct grammar
    struct GrammarCorrection {
        bool has_errors;
        std::vector<GrammarError> errors;
        std::string corrected_text;
    };
    
    GrammarCorrection check_and_correct_grammar(const std::string& text);
    
    // ========== INTEGRATED THINKING & COMMUNICATION ==========
    
    /// Process user input: understand language, analyze meaning, think about response
    struct ProcessedInput {
        MultiLanguageProcessor::Language detected_language;
        EnglishAnalysis english_analysis;  // Always provide English analysis
        std::string understood_meaning;
        float understanding_confidence;
        std::vector<std::string> key_topics;
    };
    
    ProcessedInput understand_user_input(const std::string& user_text);
    
    /// Generate intelligent multilingual response
    struct GeneratedResponse {
        std::string response_text;
        MultiLanguageProcessor::Language response_language;
        float quality_score;
        bool grammatically_correct;
        std::string reasoning;
    };
    
    GeneratedResponse generate_intelligent_response(const ProcessedInput& input,
                                                   const std::string& response_language = "en");
    
    // ========== INTERNAL STATE MANAGEMENT ==========
    
    void sleep_and_consolidate();  // Simulate sleep
    void increase_cognitive_load(float amount);
    void reset_cognitive_state();
    void enable_verbose_logging(bool enable);
    
private:
    // Component instances
    std::unique_ptr<CognitiveProcessor> cognitive_processor_;
    std::unique_ptr<EnglishProcessor> english_processor_;
    std::unique_ptr<MultiLanguageProcessor> multilingual_processor_;
    std::unique_ptr<GrammarAnalyzer> grammar_analyzer_;
    std::unique_ptr<LinguisticAnalyzer> linguistic_analyzer_;
    
    // State tracking
    bool verbose_logging_;
    std::map<std::string, int> thought_frequency_;
    std::vector<std::string> recent_topics_;
    
    // Helper methods
    void log_cognitive_activity(const std::string& activity);
    float calculate_understanding_confidence(const std::string& text);
};

} // namespace BrainLLM

#endif // INTEGRATED_COGNITIVE_MODULE_H
