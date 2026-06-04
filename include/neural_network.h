#pragma once

#include "brain_types.h"
#include <random>

namespace BrainLLM {

class NeuralNetwork {
public:
    NeuralNetwork(const BrainConfig& config);
    ~NeuralNetwork() = default;
    
    // Forward propagation
    Activation forward(const Activation& input);
    
    // Backward propagation
    void backward(const Activation& gradient);
    
    // Layer management
    void add_layer(int size);
    void initialize_weights();
    
    // Activation functions
    static float relu(float x);
    static float sigmoid(float x);
    static float tanh_activation(float x);
    
    // Gradient descent
    void update_weights(float learning_rate);
    
    // State management
    BrainMetrics get_metrics() const;
    void reset();
    
private:
    BrainConfig config_;
    std::vector<NeuralLayer> layers_;
    std::vector<NeuralLayer> gradients_;
    std::mt19937 rng_;
    
    float compute_loss(const Activation& predicted, const Activation& expected);
};

} // namespace BrainLLM
