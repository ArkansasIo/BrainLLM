#include "quantum_computing.h"
#include "quantum_llm_integration.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

bool require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        return false;
    }
    std::cout << "PASS: " << message << '\n';
    return true;
}

} // namespace

int main() {
    using namespace BrainLLM;

    bool ok = true;

    Qubit zero;
    ok &= require(std::fabs(zero.prob_zero() - 1.0f) < 0.0001f, "default qubit starts in |0>");
    ok &= require(std::fabs(zero.prob_one()) < 0.0001f, "default qubit has no |1> probability");

    QuantumGate hadamard(QuantumGate::Hadamard);
    const Qubit superposed = hadamard.apply(zero);
    ok &= require(std::fabs(superposed.prob_zero() - 0.5f) < 0.01f, "Hadamard creates balanced |0> probability");
    ok &= require(std::fabs(superposed.prob_one() - 0.5f) < 0.01f, "Hadamard creates balanced |1> probability");

    QuantumRegister reg(3);
    QuantumCircuit circuit(3);
    circuit.add_gate(QuantumGate(QuantumGate::Hadamard), 0);
    circuit.add_controlled_gate(ControlledGate(ControlledGate::CNOT), 0, 1);
    const std::vector<int> measurements = circuit.execute(reg);
    ok &= require(measurements.size() == 3, "quantum circuit returns one measurement per qubit");
    ok &= require(circuit.num_gates() == 2, "quantum circuit records gate count");

    GroversAlgorithm grover(10);
    ok &= require(grover.get_iterations_needed(1000) > 0, "Grover iteration estimate is positive");
    ok &= require(grover.get_iterations_needed(1000) < 1000, "Grover iteration estimate beats linear scan");

    QuantumLLMIntegration quantum(8);
    const auto encoding = quantum.encode_text_quantum("hello quantum assistant");
    ok &= require(!encoding.encoded_tokens.empty(), "quantum text encoding extracts tokens");
    ok &= require(encoding.quantum_state.size() == 8, "quantum text encoding uses configured qubit count");

    const std::vector<std::string> docs = {
        "classical neural language model",
        "quantum search with Grover acceleration",
        "voice assistant wake word routing"
    };
    const auto search = quantum.quantum_semantic_search("quantum Grover", docs);
    ok &= require(!search.matches.empty(), "quantum semantic search returns matches");
    ok &= require(search.iterations_used > 0, "quantum semantic search reports iterations");

    const auto features = quantum.extract_quantum_features({0.1f, 0.2f, 0.3f});
    ok &= require(!features.quantum_features.empty(), "quantum feature extraction returns quantum features");
    ok &= require(features.dimensionality_increase > 0, "quantum feature extraction reports dimensionality increase");

    const auto response = quantum.generate_response_quantum("explain quantum search");
    ok &= require(!response.primary_response.empty(), "quantum response generation returns primary response");
    ok &= require(response.relevance_score >= 0.0f, "quantum response generation reports non-negative relevance");

    return ok ? 0 : 1;
}
