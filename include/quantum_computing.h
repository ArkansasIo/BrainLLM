#ifndef QUANTUM_COMPUTING_H
#define QUANTUM_COMPUTING_H

#include <vector>
#include <complex>
#include <map>
#include <string>
#include <cmath>
#include <memory>

namespace BrainLLM {

/**
 * QuantumComputing Module
 * 
 * Integrates quantum computing with classical AI for enhanced processing.
 * Implements quantum gates, circuits, and quantum machine learning algorithms.
 */

// ========== QUANTUM FUNDAMENTALS ==========

/**
 * Quantum bit (Qubit) - superposition of 0 and 1
 * Represented as |ψ⟩ = α|0⟩ + β|1⟩
 * where |α|² + |β|² = 1
 */
struct Qubit {
    std::complex<float> alpha;      // Amplitude for |0⟩ state
    std::complex<float> beta;       // Amplitude for |1⟩ state
    
    Qubit() : alpha(1.0f, 0.0f), beta(0.0f, 0.0f) {}
    explicit Qubit(std::complex<float> a, std::complex<float> b) 
        : alpha(a), beta(b) {}
    
    // Measurement: collapse to |0⟩ or |1⟩
    int measure() const;
    
    // Probability of measuring |0⟩
    float prob_zero() const { return std::norm(alpha); }
    
    // Probability of measuring |1⟩
    float prob_one() const { return std::norm(beta); }
};

/**
 * Quantum Register - collection of qubits
 * n qubits can represent 2^n states simultaneously
 */
class QuantumRegister {
public:
    explicit QuantumRegister(int num_qubits);
    
    int size() const { return qubits_.size(); }
    Qubit get_qubit(int index) const { return qubits_[index]; }
    void set_qubit(int index, const Qubit& q) { qubits_[index] = q; }
    
    // Collapse all qubits and measure
    std::vector<int> measure_all();
    
    // Get superposition state vector
    std::vector<std::complex<float>> get_state_vector() const;
    
private:
    std::vector<Qubit> qubits_;
};

// ========== QUANTUM GATES ==========

/**
 * Single-qubit gates
 */
class QuantumGate {
public:
    enum GateType {
        Pauli_X,        // NOT gate
        Pauli_Y,        // Y gate
        Pauli_Z,        // Z gate (phase flip)
        Hadamard,       // Superposition gate
        Phase,          // Phase gate
        T,              // T gate
        S,              // S gate
        RX,             // Rotation X-axis
        RY,             // Rotation Y-axis
        RZ,             // Rotation Z-axis
        Custom          // Custom unitary
    };
    
    QuantumGate(GateType type, float theta = 0.0f);
    
    // Apply gate to single qubit
    Qubit apply(const Qubit& q) const;
    
    // Get 2x2 unitary matrix representation
    std::vector<std::vector<std::complex<float>>> get_matrix() const;
    
    GateType get_type() const { return type_; }
    
private:
    GateType type_;
    float theta_;
    std::vector<std::vector<std::complex<float>>> matrix_;
    
    void initialize_matrix();
};

/**
 * Two-qubit gates (entanglement gates)
 */
class ControlledGate {
public:
    enum Type {
        CNOT,           // Controlled NOT
        CZ,             // Controlled Z
        SWAP,           // Swap qubits
        CPHASE          // Controlled phase
    };
    
    ControlledGate(Type type, float angle = 0.0f);
    
    // Apply to two qubits
    void apply(QuantumRegister& reg, int control, int target);
    
    Type get_type() const { return type_; }
    
private:
    Type type_;
    float angle_;
};

// ========== QUANTUM CIRCUITS ==========

/**
 * Quantum circuit - sequence of quantum gates
 */
class QuantumCircuit {
public:
    explicit QuantumCircuit(int num_qubits);
    
    int num_qubits() const { return num_qubits_; }
    int num_gates() const { return gates_.size(); }
    
    // Add single-qubit gate
    void add_gate(const QuantumGate& gate, int qubit);
    
    // Add two-qubit gate
    void add_controlled_gate(const ControlledGate& gate, int control, int target);
    
    // Apply all gates in sequence
    std::vector<int> execute(QuantumRegister& reg);
    
    // Get circuit depth (longest path)
    int get_depth() const { return depth_; }
    
    // Get circuit description
    std::string to_string() const;
    
private:
    struct GateOperation {
        std::shared_ptr<QuantumGate> gate;
        int qubit1;
        int qubit2;
        bool is_controlled;
    };
    
    int num_qubits_;
    int depth_;
    std::vector<GateOperation> gates_;
};

// ========== QUANTUM ALGORITHMS ==========

/**
 * Grover's Algorithm - quantum search
 * Provides quadratic speedup for unstructured search
 */
class GroversAlgorithm {
public:
    explicit GroversAlgorithm(int num_qubits);
    
    // Search for marked item in database
    std::vector<int> search(const std::vector<std::string>& database, 
                           const std::string& target);
    
    // Get number of iterations needed
    int get_iterations_needed(int database_size) const;
    
private:
    int num_qubits_;
    
    // Oracle: marks the target state
    void apply_oracle(QuantumCircuit& circuit, const std::string& target);
    
    // Diffusion operator
    void apply_diffusion(QuantumCircuit& circuit);
};

/**
 * Quantum Fourier Transform (QFT)
 * Efficient computation of discrete Fourier transform
 */
class QuantumFourierTransform {
public:
    explicit QuantumFourierTransform(int num_qubits);
    
    // Apply QFT to quantum register
    std::vector<std::complex<float>> transform(QuantumRegister& reg);
    
    // Get complexity advantage over classical FFT
    float speedup_factor() const;
    
private:
    int num_qubits_;
    
    void apply_phase_gates(QuantumCircuit& circuit, int qubit);
    void apply_hadamard_cascade(QuantumCircuit& circuit);
};

/**
 * Phase Estimation - core for many quantum algorithms
 */
class PhaseEstimation {
public:
    explicit PhaseEstimation(int num_qubits);
    
    // Estimate eigenvalue phase
    float estimate_phase(const std::vector<std::complex<float>>& eigenvalues);
    
private:
    int num_qubits_;
};

// ========== QUANTUM MACHINE LEARNING ==========

/**
 * Quantum Neural Network Layer
 * Uses parameterized quantum circuits as neurons
 */
class QuantumNeuralLayer {
public:
    QuantumNeuralLayer(int num_qubits, int num_parameters);
    
    // Forward pass through quantum layer
    std::vector<float> forward(const std::vector<float>& input, 
                              const std::vector<float>& parameters);
    
    // Compute gradient using parameter shift rule
    std::vector<float> compute_gradient(const std::vector<float>& input,
                                       const std::vector<float>& parameters);
    
    int num_qubits() const { return num_qubits_; }
    int num_parameters() const { return num_parameters_; }
    
private:
    int num_qubits_;
    int num_parameters_;
    
    // Parameterized quantum circuit
    std::shared_ptr<QuantumCircuit> circuit_;
};

/**
 * Variational Quantum Eigensolver (VQE)
 * Find minimum eigenvalue of a Hamiltonian
 */
class VariationalQuantumEigensolver {
public:
    explicit VariationalQuantumEigensolver(int num_qubits);
    
    // Solve for ground state energy
    float solve(const std::vector<std::complex<float>>& hamiltonian);
    
    // Get optimized parameters
    std::vector<float> get_parameters() const { return parameters_; }
    
private:
    int num_qubits_;
    std::vector<float> parameters_;
    
    // Cost function to minimize
    float cost_function(const std::vector<float>& params,
                       const std::vector<std::complex<float>>& hamiltonian);
};

/**
 * Quantum Support Vector Machine (QSVM)
 * Use quantum kernel for classification
 */
class QuantumSVM {
public:
    QuantumSVM(int num_qubits, int num_features);
    
    // Train on quantum-mapped data
    void train(const std::vector<std::vector<float>>& features,
              const std::vector<int>& labels);
    
    // Classify using quantum kernel
    std::vector<int> predict(const std::vector<std::vector<float>>& features);
    
    // Compute quantum kernel
    float quantum_kernel(const std::vector<float>& x1, 
                        const std::vector<float>& x2);
    
private:
    int num_qubits_;
    int num_features_;
    std::vector<std::pair<std::vector<float>, int>> training_data_;
};

// ========== QUANTUM-CLASSICAL HYBRID ==========

/**
 * Hybrid Quantum-Classical Processor
 * Combines quantum algorithms with classical neural networks
 */
class HybridQuantumClassical {
public:
    HybridQuantumClassical(int num_qubits, int classical_neurons);
    
    // Process input through quantum + classical layers
    std::vector<float> process(const std::vector<float>& input,
                              const std::vector<float>& quantum_params,
                              const std::vector<float>& classical_weights);
    
    // Quantum layer outputs expectation values
    std::vector<float> quantum_forward(const std::vector<float>& input,
                                      const std::vector<float>& parameters);
    
    // Classical neural layer
    std::vector<float> classical_forward(const std::vector<float>& input,
                                        const std::vector<float>& weights);
    
private:
    std::unique_ptr<QuantumNeuralLayer> quantum_layer_;
    int classical_neurons_;
};

/**
 * Quantum Parameter Optimization
 * Finds optimal parameters for quantum circuits
 */
class QuantumOptimizer {
public:
    enum OptimizationMethod {
        GradientDescent,
        NesterovMomentum,
        Adam,
        COBYLA,
        SPSA
    };
    
    QuantumOptimizer(OptimizationMethod method, float learning_rate = 0.01f);
    
    // Optimize quantum circuit parameters
    std::vector<float> optimize(
        const std::vector<float>& initial_params,
        int num_iterations,
        std::function<float(const std::vector<float>&)> cost_function
    );
    
private:
    OptimizationMethod method_;
    float learning_rate_;
    float momentum_ = 0.9f;
    
    // Adam optimizer state
    struct AdamState {
        std::vector<float> m;  // First moment
        std::vector<float> v;  // Second moment
        int t = 0;             // Time step
    } adam_state_;
};

// ========== QUANTUM ERROR CORRECTION ==========

/**
 * Quantum Error Detection and Correction
 * Protects quantum information from decoherence
 */
class QuantumErrorCorrection {
public:
    enum ErrorType {
        BitFlip,        // X error
        PhaseFlip,      // Z error
        BothFlip        // Y error
    };
    
    // Encode logical qubit with error correction
    QuantumRegister encode(const Qubit& logical_qubit);
    
    // Detect and correct errors
    Qubit decode(const QuantumRegister& encoded_register);
    
    // Estimate error rate
    float estimate_error_rate();
    
private:
    // Surface code parameters
    int code_distance_ = 3;
    float error_probability_ = 0.001f;
};

// ========== QUANTUM-CLASSICAL INTERFACE ==========

/**
 * Quantum Measurement & State Tomography
 * Reconstruct quantum state from measurements
 */
class QuantumStateTomography {
public:
    // Single qubit state tomography
    Qubit reconstruct_single_qubit(const std::vector<int>& measurement_results,
                                  int num_measurements);
    
    // Multi-qubit state tomography
    std::vector<std::complex<float>> reconstruct_state_vector(
        const std::vector<std::vector<int>>& measurement_results,
        int num_measurements
    );
    
    // Fidelity between two quantum states
    float fidelity(const std::vector<std::complex<float>>& state1,
                  const std::vector<std::complex<float>>& state2);
    
private:
    float estimate_expectation(const std::vector<int>& results);
};

} // namespace BrainLLM

#endif // QUANTUM_COMPUTING_H
