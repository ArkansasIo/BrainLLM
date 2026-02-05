#pragma once

#include "brain_types.h"
#include <vector>
#include <string>
#include <map>

namespace BrainLLM {

// ========================================
// ADVANCED LEARNING SYSTEMS
// ========================================

enum class LearningMode {
    Supervised,         // Learning from labeled data
    Unsupervised,       // Finding patterns in unlabeled data
    Reinforcement,      // Learning from rewards/penalties
    Transfer,           // Applying knowledge from one domain to another
    MetaLearning,       // Learning how to learn
    ContinuousLearning  // Ongoing learning from experience
};

struct LearningSession {
    uint64_t session_id;
    LearningMode mode;
    std::string dataset_name;
    int epochs;
    float initial_accuracy;
    float final_accuracy;
    float learning_rate;
    std::vector<float> loss_history;
    std::vector<float> accuracy_history;
};

struct ReinforcementReward {
    std::string action;
    float reward;
    int timestamp;
    std::string context;
};

class AdvancedLearning {
public:
    AdvancedLearning();
    ~AdvancedLearning() = default;
    
    // Supervised Learning
    void train_supervised(const std::vector<std::string>& inputs,
                         const std::vector<std::string>& labels);
    float evaluate_supervised(const std::vector<std::string>& test_inputs);
    
    // Unsupervised Learning
    std::vector<std::vector<std::string>> cluster_data(
        const std::vector<std::string>& data, int num_clusters);
    
    // Reinforcement Learning
    void apply_reinforcement(const ReinforcementReward& reward);
    void learn_from_feedback(const std::string& action, float reward);
    float get_q_value(const std::string& state, const std::string& action) const;
    
    // Transfer Learning
    void transfer_knowledge_from(const std::string& source_domain,
                                const std::string& target_domain);
    
    // Meta-learning
    void learn_learning_strategy();
    float predict_learnability(const std::string& task) const;
    
    // Continuous Learning
    void continuous_update(const std::string& experience);
    void consolidate_learning();
    
    // Monitoring
    LearningSession get_current_session() const;
    std::vector<LearningSession> get_learning_history() const;
    float get_overall_accuracy() const;
    float get_learning_efficiency() const;
    
private:
    std::map<std::string, float> q_table_;  // Q-learning table
    std::vector<LearningSession> session_history_;
    LearningSession current_session_;
    float cumulative_accuracy_;
    int total_training_steps_;
};

} // namespace BrainLLM
