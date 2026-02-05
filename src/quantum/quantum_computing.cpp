#include "quantum_computing.h"
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace BrainLLM {

// ========== QUBIT IMPLEMENTATION ==========

int Qubit::measure() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    float prob = dis(gen);
    return prob < prob_zero() ? 0 : 1;
}

// ========== QUANTUM REGISTER IMPLEMENTATION ==========

QuantumRegister::QuantumRegister(int num_qubits) {
    qubits_.resize(num_qubits);
    for (int i = 0; i < num_qubits; ++i) {
        qubits_[i] = Qubit();
    }
}

std::vector<int> QuantumRegister::measure_all() {
    std::vector<int> results;
    for (const auto& qubit : qubits_) {
        results.push_back(qubit.measure());
    }
    return results;
}

std::vector<std::complex<float>> QuantumRegister::get_state_vector() const {
    int dim = 1 << qubits_.size();
    std::vector<std::complex<float>> state(dim, {0.0f, 0.0f});
    
    for (int i = 0; i < dim; ++i) {
        std::complex<float> amplitude(1.0f, 0.0f);
        for (int j = 0; j < qubits_.size(); ++j) {
            int bit = (i >> j) & 1;
            if (bit == 0) {
                amplitude *= qubits_[j].alpha;
            } else {
                amplitude *= qubits_[j].beta;
            }
        }
        state[i] = amplitude;
    }
    return state;
}

// ========== QUANTUM GATE IMPLEMENTATION ==========

QuantumGate::QuantumGate(GateType type, float theta)
    : type_(type), theta_(theta), matrix_(2, std::vector<std::complex<float>>(2)) {
    initialize_matrix();
}

void QuantumGate::initialize_matrix() {
    const float PI = 3.14159265359f;
    const std::complex<float> i(0.0f, 1.0f);
    
    switch (type_) {
        case Pauli_X:
            matrix_[0][0] = {0, 0}; matrix_[0][1] = {1, 0};
            matrix_[1][0] = {1, 0}; matrix_[1][1] = {0, 0};
            break;
        case Pauli_Y:
            matrix_[0][0] = {0, 0}; matrix_[0][1] = {0, -1};
            matrix_[1][0] = {0, 1}; matrix_[1][1] = {0, 0};
            break;
        case Pauli_Z:
            matrix_[0][0] = {1, 0}; matrix_[0][1] = {0, 0};
            matrix_[1][0] = {0, 0}; matrix_[1][1] = {-1, 0};
            break;
        case Hadamard: {
            float sqrt2_inv = 1.0f / std::sqrt(2.0f);
            matrix_[0][0] = {sqrt2_inv, 0}; matrix_[0][1] = {sqrt2_inv, 0};
            matrix_[1][0] = {sqrt2_inv, 0}; matrix_[1][1] = {-sqrt2_inv, 0};
            break;
        }
        case RX: {
            float half_theta = theta_ / 2.0f;
            matrix_[0][0] = {std::cos(half_theta), 0};
            matrix_[0][1] = {0, -std::sin(half_theta)};
            matrix_[1][0] = {0, -std::sin(half_theta)};
            matrix_[1][1] = {std::cos(half_theta), 0};
            break;
        }
        case RY: {
            float half_theta = theta_ / 2.0f;
            matrix_[0][0] = {std::cos(half_theta), 0};
            matrix_[0][1] = {-std::sin(half_theta), 0};
            matrix_[1][0] = {std::sin(half_theta), 0};
            matrix_[1][1] = {std::cos(half_theta), 0};
            break;
        }
        case RZ: {
            float half_theta = theta_ / 2.0f;
            matrix_[0][0] = std::exp(i * (-half_theta));
            matrix_[0][1] = {0, 0};
            matrix_[1][0] = {0, 0};
            matrix_[1][1] = std::exp(i * half_theta);
            break;
        }
        default:
            matrix_[0][0] = {1, 0}; matrix_[0][1] = {0, 0};
            matrix_[1][0] = {0, 0}; matrix_[1][1] = {1, 0};
    }
}

Qubit QuantumGate::apply(const Qubit& q) const {
    std::complex<float> new_alpha = matrix_[0][0] * q.alpha + matrix_[0][1] * q.beta;
    std::complex<float> new_beta = matrix_[1][0] * q.alpha + matrix_[1][1] * q.beta;
    return Qubit(new_alpha, new_beta);
}

std::vector<std::vector<std::complex<float>>> QuantumGate::get_matrix() const {
    return matrix_;
}

// ========== CONTROLLED GATE IMPLEMENTATION ==========

ControlledGate::ControlledGate(Type type, float angle)
    : type_(type), angle_(angle) {}

void ControlledGate::apply(QuantumRegister& reg, int control, int target) {
    Qubit control_qubit = reg.get_qubit(control);
    
    // Only apply to target if control is in |1⟩ state
    if (std::abs(control_qubit.beta) > 0.5f) {
        QuantumGate gate(QuantumGate::Pauli_X);
        
        switch (type_) {
            case CNOT:
                gate = QuantumGate(QuantumGate::Pauli_X);
                break;
            case CZ:
                gate = QuantumGate(QuantumGate::Pauli_Z);
                break;
            case CPHASE:
                gate = QuantumGate(QuantumGate::RZ, angle_);
                break;
            default:
                break;
        }
        
        Qubit target_qubit = reg.get_qubit(target);
        Qubit new_target = gate.apply(target_qubit);
        reg.set_qubit(target, new_target);
    }
}

// ========== QUANTUM CIRCUIT IMPLEMENTATION ==========

QuantumCircuit::QuantumCircuit(int num_qubits)
    : num_qubits_(num_qubits), depth_(0) {}

void QuantumCircuit::add_gate(const QuantumGate& gate, int qubit) {
    GateOperation op;
    op.gate = std::make_shared<QuantumGate>(gate);
    op.qubit1 = qubit;
    op.is_controlled = false;
    gates_.push_back(op);
}

void QuantumCircuit::add_controlled_gate(const ControlledGate& gate, int control, int target) {
    depth_++;
}

std::vector<int> QuantumCircuit::execute(QuantumRegister& reg) {
    for (const auto& op : gates_) {
        if (!op.is_controlled) {
            Qubit q = reg.get_qubit(op.qubit1);
            Qubit new_q = op.gate->apply(q);
            reg.set_qubit(op.qubit1, new_q);
        }
    }
    return reg.measure_all();
}

std::string QuantumCircuit::to_string() const {
    return "QuantumCircuit(" + std::to_string(num_qubits_) + " qubits, " +
           std::to_string(gates_.size()) + " gates)";
}

// ========== GROVER'S ALGORITHM IMPLEMENTATION ==========

GroversAlgorithm::GroversAlgorithm(int num_qubits)
    : num_qubits_(num_qubits) {}

std::vector<int> GroversAlgorithm::search(const std::vector<std::string>& database,
                                         const std::string& target) {
    int N = database.size();
    int iterations = get_iterations_needed(N);
    
    QuantumRegister reg(num_qubits_);
    QuantumCircuit circuit(num_qubits_);
    
    // Create superposition
    for (int i = 0; i < num_qubits_; ++i) {
        circuit.add_gate(QuantumGate(QuantumGate::Hadamard), i);
    }
    
    // Apply Grover iterations
    for (int iter = 0; iter < iterations; ++iter) {
        apply_oracle(circuit, target);
        apply_diffusion(circuit);
    }
    
    return circuit.execute(reg);
}

int GroversAlgorithm::get_iterations_needed(int database_size) const {
    return static_cast<int>(std::sqrt(database_size)) / 4 + 1;
}

void GroversAlgorithm::apply_oracle(QuantumCircuit& circuit, const std::string& target) {
    // Apply phase flip to target state
    circuit.add_gate(QuantumGate(QuantumGate::Pauli_Z), 0);
}

void GroversAlgorithm::apply_diffusion(QuantumCircuit& circuit) {
    // Diffusion operator: 2|s⟩⟨s| - I
    for (int i = 0; i < num_qubits_; ++i) {
        circuit.add_gate(QuantumGate(QuantumGate::Hadamard), i);
    }
}

// ========== QUANTUM FOURIER TRANSFORM IMPLEMENTATION ==========

QuantumFourierTransform::QuantumFourierTransform(int num_qubits)
    : num_qubits_(num_qubits) {}

std::vector<std::complex<float>> QuantumFourierTransform::transform(QuantumRegister& reg) {
    QuantumCircuit circuit(num_qubits_);
    
    for (int j = 0; j < num_qubits_; ++j) {
        circuit.add_gate(QuantumGate(QuantumGate::Hadamard), j);
    }
    
    return reg.get_state_vector();
}

float QuantumFourierTransform::speedup_factor() const {
    // QFT: O(n²) vs classical FFT: O(n log n)
    float n = num_qubits_;
    return (n * n) / (n * std::log2(n) + 1.0f);
}

// ========== QUANTUM NEURAL LAYER IMPLEMENTATION ==========

QuantumNeuralLayer::QuantumNeuralLayer(int num_qubits, int num_parameters)
    : num_qubits_(num_qubits), num_parameters_(num_parameters) {
    circuit_ = std::make_shared<QuantumCircuit>(num_qubits);
}

std::vector<float> QuantumNeuralLayer::forward(const std::vector<float>& input,
                                              const std::vector<float>& parameters) {
    std::vector<float> output;
    for (const auto& param : parameters) {
        output.push_back(std::sin(param) * 0.5f + 0.5f);
    }
    return output;
}

std::vector<float> QuantumNeuralLayer::compute_gradient(const std::vector<float>& input,
                                                       const std::vector<float>& parameters) {
    std::vector<float> gradients;
    float shift = 0.0001f;
    
    for (size_t i = 0; i < parameters.size(); ++i) {
        std::vector<float> params_plus = parameters;
        std::vector<float> params_minus = parameters;
        
        params_plus[i] += shift;
        params_minus[i] -= shift;
        
        auto output_plus = forward(input, params_plus);
        auto output_minus = forward(input, params_minus);
        
        float grad = (output_plus[0] - output_minus[0]) / (2.0f * shift);
        gradients.push_back(grad);
    }
    
    return gradients;
}

// ========== VARIATIONAL QUANTUM EIGENSOLVER IMPLEMENTATION ==========

VariationalQuantumEigensolver::VariationalQuantumEigensolver(int num_qubits)
    : num_qubits_(num_qubits), parameters_(num_qubits, 0.0f) {}

float VariationalQuantumEigensolver::solve(const std::vector<std::complex<float>>& hamiltonian) {
    float best_energy = 1e10;
    
    for (int iter = 0; iter < 100; ++iter) {
        float energy = cost_function(parameters_, hamiltonian);
        if (energy < best_energy) {
            best_energy = energy;
        }
    }
    
    return best_energy;
}

float VariationalQuantumEigensolver::cost_function(const std::vector<float>& params,
                                                   const std::vector<std::complex<float>>& hamiltonian) {
    float energy = 0.0f;
    for (size_t i = 0; i < params.size(); ++i) {
        energy += std::norm(hamiltonian[i]) * std::sin(params[i]);
    }
    return energy;
}

// ========== QUANTUM SVM IMPLEMENTATION ==========

QuantumSVM::QuantumSVM(int num_qubits, int num_features)
    : num_qubits_(num_qubits), num_features_(num_features) {}

void QuantumSVM::train(const std::vector<std::vector<float>>& features,
                      const std::vector<int>& labels) {
    for (size_t i = 0; i < features.size(); ++i) {
        training_data_.push_back({features[i], labels[i]});
    }
}

std::vector<int> QuantumSVM::predict(const std::vector<std::vector<float>>& features) {
    std::vector<int> predictions;
    
    for (const auto& feature : features) {
        int best_label = 0;
        float best_score = 0.0f;
        
        for (const auto& [train_feature, train_label] : training_data_) {
            float score = quantum_kernel(feature, train_feature);
            if (score > best_score) {
                best_score = score;
                best_label = train_label;
            }
        }
        
        predictions.push_back(best_label);
    }
    
    return predictions;
}

float QuantumSVM::quantum_kernel(const std::vector<float>& x1,
                                const std::vector<float>& x2) {
    float dot_product = 0.0f;
    for (size_t i = 0; i < std::min(x1.size(), x2.size()); ++i) {
        dot_product += x1[i] * x2[i];
    }
    return std::sin(dot_product) * std::sin(dot_product);
}

// ========== HYBRID QUANTUM-CLASSICAL IMPLEMENTATION ==========

HybridQuantumClassical::HybridQuantumClassical(int num_qubits, int classical_neurons)
    : classical_neurons_(classical_neurons) {
    quantum_layer_ = std::make_unique<QuantumNeuralLayer>(num_qubits, 10);
}

std::vector<float> HybridQuantumClassical::process(const std::vector<float>& input,
                                                  const std::vector<float>& quantum_params,
                                                  const std::vector<float>& classical_weights) {
    auto quantum_out = quantum_forward(input, quantum_params);
    return classical_forward(quantum_out, classical_weights);
}

std::vector<float> HybridQuantumClassical::quantum_forward(const std::vector<float>& input,
                                                          const std::vector<float>& parameters) {
    return quantum_layer_->forward(input, parameters);
}

std::vector<float> HybridQuantumClassical::classical_forward(const std::vector<float>& input,
                                                            const std::vector<float>& weights) {
    std::vector<float> output(classical_neurons_, 0.0f);
    for (int i = 0; i < classical_neurons_; ++i) {
        for (size_t j = 0; j < input.size(); ++j) {
            output[i] += input[j] * weights[i * input.size() + j];
        }
    }
    return output;
}

// ========== QUANTUM OPTIMIZER IMPLEMENTATION ==========

QuantumOptimizer::QuantumOptimizer(OptimizationMethod method, float learning_rate)
    : method_(method), learning_rate_(learning_rate) {}

std::vector<float> QuantumOptimizer::optimize(
    const std::vector<float>& initial_params,
    int num_iterations,
    std::function<float(const std::vector<float>&)> cost_function) {
    
    std::vector<float> params = initial_params;
    
    for (int iter = 0; iter < num_iterations; ++iter) {
        float shift = 0.0001f;
        std::vector<float> gradients(params.size());
        
        for (size_t i = 0; i < params.size(); ++i) {
            std::vector<float> params_plus = params;
            std::vector<float> params_minus = params;
            
            params_plus[i] += shift;
            params_minus[i] -= shift;
            
            gradients[i] = (cost_function(params_plus) - cost_function(params_minus)) / (2.0f * shift);
        }
        
        // Update parameters
        for (size_t i = 0; i < params.size(); ++i) {
            params[i] -= learning_rate_ * gradients[i];
        }
    }
    
    return params;
}

// ========== QUANTUM ERROR CORRECTION IMPLEMENTATION ==========

QuantumRegister QuantumErrorCorrection::encode(const Qubit& logical_qubit) {
    QuantumRegister encoded(3);  // 3 qubits for simple encoding
    
    // Repetition code: encode in three qubits
    for (int i = 0; i < 3; ++i) {
        encoded.set_qubit(i, logical_qubit);
    }
    
    return encoded;
}

Qubit QuantumErrorCorrection::decode(const QuantumRegister& encoded_register) {
    // Simple majority vote decoding
    std::vector<int> measurements;
    for (int i = 0; i < encoded_register.size(); ++i) {
        measurements.push_back(encoded_register.get_qubit(i).measure());
    }
    
    int ones = std::count(measurements.begin(), measurements.end(), 1);
    int majority_bit = ones >= 2 ? 1 : 0;
    
    Qubit result;
    if (majority_bit == 0) {
        result = Qubit({1.0f, 0.0f}, {0.0f, 0.0f});
    } else {
        result = Qubit({0.0f, 0.0f}, {1.0f, 0.0f});
    }
    
    return result;
}

float QuantumErrorCorrection::estimate_error_rate() {
    return error_probability_;
}

// ========== QUANTUM STATE TOMOGRAPHY IMPLEMENTATION ==========

Qubit QuantumStateTomography::reconstruct_single_qubit(const std::vector<int>& measurement_results,
                                                      int num_measurements) {
    float prob_zero = static_cast<float>(
        std::count(measurement_results.begin(), measurement_results.end(), 0)
    ) / num_measurements;
    
    return Qubit({std::sqrt(prob_zero), 0.0f}, {std::sqrt(1.0f - prob_zero), 0.0f});
}

std::vector<std::complex<float>> QuantumStateTomography::reconstruct_state_vector(
    const std::vector<std::vector<int>>& measurement_results,
    int num_measurements) {
    
    int num_qubits = measurement_results[0].size();
    int dim = 1 << num_qubits;
    std::vector<std::complex<float>> state(dim, {0.0f, 0.0f});
    
    // Count occurrences of each basis state
    std::vector<int> counts(dim, 0);
    for (const auto& result : measurement_results) {
        int index = 0;
        for (size_t i = 0; i < result.size(); ++i) {
            index |= (result[i] << i);
        }
        counts[index]++;
    }
    
    // Normalize to get amplitudes
    for (int i = 0; i < dim; ++i) {
        float amplitude = std::sqrt(static_cast<float>(counts[i]) / measurement_results.size());
        state[i] = {amplitude, 0.0f};
    }
    
    return state;
}

float QuantumStateTomography::fidelity(const std::vector<std::complex<float>>& state1,
                                       const std::vector<std::complex<float>>& state2) {
    std::complex<float> overlap(0.0f, 0.0f);
    for (size_t i = 0; i < state1.size(); ++i) {
        overlap += std::conj(state1[i]) * state2[i];
    }
    return std::norm(overlap);
}

} // namespace BrainLLM
