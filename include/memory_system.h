#pragma once

#include "brain_types.h"
#include <queue>
#include <deque>

namespace BrainLLM {

class MemorySystem {
public:
    MemorySystem(int max_size = 10000);
    ~MemorySystem() = default;
    
    // Memory operations
    void store_memory(const std::string& content, float importance = 1.0f);
    std::vector<MemoryRecord> retrieve_memories(const std::string& query, int count = 5);
    void clear_memories();
    
    // Memory management
    int get_memory_count() const;
    float get_memory_usage() const;
    void consolidate_memories();
    void decay_old_memories();
    
    // Categories
    void categorize_memory(const std::string& content, const std::string& category);
    std::vector<MemoryRecord> get_memories_by_category(const std::string& category);
    
private:
    struct MemoryEntry {
        MemoryRecord record;
        uint64_t last_accessed;
    };
    
    std::deque<MemoryEntry> memory_storage_;
    int max_size_;
    
    float calculate_relevance(const std::string& memory, const std::string& query) const;
    void remove_oldest();
};

} // namespace BrainLLM
