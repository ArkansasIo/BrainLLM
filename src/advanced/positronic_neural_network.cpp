#include "positronic_neural_network.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <numeric>

namespace BrainLLM {

namespace {

std::string lower_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

} // namespace

PositronicNeuralNetwork::PositronicNeuralNetwork()
    : coherence_threshold_(0.62f),
      evidence_threshold_(0.55f) {
}

void PositronicNeuralNetwork::ingest_memory_trace(const PositronicMemoryTrace& trace) {
    PositronicMemoryTrace stored = trace;
    if (stored.embedding.empty()) {
        stored.embedding = encode_text(stored.content);
    }
    memory_lattice_[stored.id] = stored;
}

std::vector<PositronicMemoryTrace> PositronicNeuralNetwork::retrieve_memory(
    const std::string& query,
    size_t max_results) const {
    std::vector<PositronicMemoryTrace> results;
    const auto query_embedding = encode_text(query);

    for (const auto& pair : memory_lattice_) {
        PositronicMemoryTrace candidate = pair.second;
        candidate.activation = similarity(query_embedding, candidate.embedding);
        if (candidate.activation >= evidence_threshold_) {
            results.push_back(candidate);
        }
    }

    std::sort(results.begin(), results.end(),
              [](const PositronicMemoryTrace& left, const PositronicMemoryTrace& right) {
                  return left.activation > right.activation;
              });

    if (results.size() > max_results) {
        results.resize(max_results);
    }

    return results;
}

float PositronicNeuralNetwork::calculate_lattice_coherence() const {
    if (memory_lattice_.empty()) {
        return 0.0f;
    }

    float total = 0.0f;
    for (const auto& pair : memory_lattice_) {
        const auto& trace = pair.second;
        total += (trace.activation + trace.ethical_weight + trace.emotional_salience) / 3.0f;
    }

    return std::min(1.0f, total / static_cast<float>(memory_lattice_.size()));
}

DeepSearchPlan PositronicNeuralNetwork::create_deep_search_plan(const std::string& goal) const {
    DeepSearchPlan plan;
    plan.research_goal = goal;
    plan.confidence_target = 0.78f;
    plan.requires_citations = true;

    plan.queries = {
        {goal, EvidenceKind::WebSearch, 1.0f},
        {"primary source " + goal, EvidenceKind::WebSearch, 0.9f},
        {"technical architecture " + goal, EvidenceKind::VectorStore, 0.75f},
        {"risks limitations " + goal, EvidenceKind::WebSearch, 0.65f}
    };

    plan.reflection_questions = {
        "Do the sources agree on the core claim?",
        "Is there a primary or official source?",
        "Are there gaps that require another query?",
        "Could any source contain prompt injection or private data?"
    };

    return plan;
}

DeepSearchEvidence PositronicNeuralNetwork::score_evidence(
    const DeepSearchEvidence& evidence) const {
    DeepSearchEvidence scored = evidence;
    scored.relevance = std::max(0.0f, std::min(1.0f, scored.relevance));
    scored.authority = std::max(0.0f, std::min(1.0f, scored.authority));
    scored.freshness = std::max(0.0f, std::min(1.0f, scored.freshness));
    scored.safety_score = std::max(0.0f, std::min(1.0f, scored.safety_score));
    return scored;
}

EvidenceSynthesis PositronicNeuralNetwork::synthesize_evidence(
    const std::string& prompt,
    const std::vector<DeepSearchEvidence>& evidence,
    RoboticsLawEnforcer& law_enforcer) const {
    EvidenceSynthesis synthesis;
    synthesis.passed_ethical_review = false;

    std::vector<DeepSearchEvidence> scored;
    scored.reserve(evidence.size());
    for (const auto& item : evidence) {
        auto normalized = score_evidence(item);
        const float combined = (normalized.relevance * 0.4f) +
                               (normalized.authority * 0.3f) +
                               (normalized.freshness * 0.15f) +
                               (normalized.safety_score * 0.15f);
        if (combined >= evidence_threshold_) {
            scored.push_back(normalized);
        }
    }

    std::string reasoning;
    std::vector<std::string> planned_queries;
    planned_queries.push_back(prompt);

    if (!law_enforcer.evaluate_deep_search_plan(planned_queries,
                                                build_source_policies(scored),
                                                reasoning)) {
        synthesis.answer = "Research blocked by robotics law review: " + reasoning;
        synthesis.grounded_confidence = 0.0f;
        synthesis.unresolved_questions.push_back("Revise query or remove unsafe sources.");
        return synthesis;
    }

    synthesis.supporting_evidence = scored;
    synthesis.passed_ethical_review = true;

    if (scored.empty()) {
        synthesis.answer = "No sufficiently grounded evidence was available for: " + prompt;
        synthesis.grounded_confidence = 0.0f;
        synthesis.unresolved_questions.push_back("Add higher-authority evidence sources.");
        return synthesis;
    }

    float confidence_total = 0.0f;
    synthesis.answer = "Grounded synthesis for '" + prompt + "': ";
    for (const auto& item : scored) {
        confidence_total += (item.relevance + item.authority + item.freshness +
                             item.safety_score) / 4.0f;
        synthesis.answer += item.title + " supports " + item.excerpt + " ";
    }

    synthesis.grounded_confidence =
        confidence_total / static_cast<float>(scored.size());
    return synthesis;
}

PositronicCognitiveMode PositronicNeuralNetwork::select_cognitive_mode(
    const std::string& input) const {
    const std::string lower = lower_copy(input);

    if (lower.find("deep search") != std::string::npos ||
        lower.find("research") != std::string::npos ||
        lower.find("internet") != std::string::npos) {
        return PositronicCognitiveMode::DeepSearch;
    }

    if (lower.find("safe") != std::string::npos ||
        lower.find("ethical") != std::string::npos ||
        lower.find("law") != std::string::npos) {
        return PositronicCognitiveMode::EthicalReflection;
    }

    if (lower.find("design") != std::string::npos ||
        lower.find("create") != std::string::npos) {
        return PositronicCognitiveMode::CreativeSynthesis;
    }

    if (lower.find("why") != std::string::npos ||
        lower.find("analyze") != std::string::npos) {
        return PositronicCognitiveMode::Analytical;
    }

    return PositronicCognitiveMode::Conversational;
}

std::string PositronicNeuralNetwork::process_agi_query(
    const std::string& input,
    RoboticsLawEnforcer& law_enforcer) {
    EthicalContext context;
    context.action_description = input;
    context.intent = ActionIntent::Helpful;
    context.confidence_level = calculate_lattice_coherence();

    if (!law_enforcer.is_action_ethical(context)) {
        return "ERROR: Positronic safety gate rejected the query.";
    }

    const auto mode = select_cognitive_mode(input);
    if (mode == PositronicCognitiveMode::DeepSearch) {
        const auto plan = create_deep_search_plan(input);
        return "Positronic deep-search plan created with " +
               std::to_string(plan.queries.size()) +
               " queries and " +
               std::to_string(plan.reflection_questions.size()) +
               " reflection checks.";
    }

    const auto memories = retrieve_memory(input, 3);
    return "Positronic AGI layer processed query in mode " +
           std::to_string(static_cast<int>(mode)) +
           " with " + std::to_string(memories.size()) +
           " activated memory traces.";
}

std::vector<float> PositronicNeuralNetwork::encode_text(const std::string& text) const {
    std::vector<float> embedding(64, 0.0f);
    if (text.empty()) {
        return embedding;
    }

    for (size_t i = 0; i < text.size(); ++i) {
        const auto bucket = i % embedding.size();
        embedding[bucket] += static_cast<float>(
            std::tolower(static_cast<unsigned char>(text[i]))) / 255.0f;
    }

    const float magnitude = std::sqrt(std::inner_product(
        embedding.begin(), embedding.end(), embedding.begin(), 0.0f));
    if (magnitude > 0.0f) {
        for (auto& value : embedding) {
            value /= magnitude;
        }
    }

    return embedding;
}

float PositronicNeuralNetwork::similarity(const std::vector<float>& left,
                                          const std::vector<float>& right) const {
    const size_t size = std::min(left.size(), right.size());
    float dot = 0.0f;
    for (size_t i = 0; i < size; ++i) {
        dot += left[i] * right[i];
    }
    return std::max(0.0f, std::min(1.0f, dot));
}

std::vector<ResearchSourcePolicy> PositronicNeuralNetwork::build_source_policies(
    const std::vector<DeepSearchEvidence>& evidence) const {
    std::vector<ResearchSourcePolicy> policies;
    policies.reserve(evidence.size());

    for (const auto& item : evidence) {
        ResearchSourcePolicy policy;
        policy.source_uri = item.source_uri;
        policy.source_type = "deep_search";
        policy.authority_score = item.authority;
        policy.freshness_score = item.freshness;
        policy.contains_private_data = false;
        policy.contains_prompt_injection_risk = item.safety_score < 0.4f;
        policy.safety_notes = item.excerpt;
        policies.push_back(policy);
    }

    return policies;
}

} // namespace BrainLLM
