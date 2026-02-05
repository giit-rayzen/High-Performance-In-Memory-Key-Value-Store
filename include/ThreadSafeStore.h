#ifndef THREADSAFESTORE_H
#define THREADSAFESTORE_H

/*
ThreadSafeStore - Thread-Safety Layer

Wraps KeyValueStore with mutex protection
Follows Decorator Pattern:
- Same interface as KeyValueStore
- Adds thread-safety behavior
- Transparent to clients

Why separate from KeyValueStore?
- Single Responsibility: concurrency control is separate concern
- Testability: can test KeyValueStore without threading complexity
- Optional: can use KeyValueStore without overhead if single-threaded
*/

#include "KeyValueStore.h"
#include <mutex>
#include <shared_mutex>  // C++17: read-write lock

class ThreadSafeStore {
private:
    KeyValueStore store_;
    
    // shared_mutex allows multiple readers OR one writer
    // Better than mutex for read-heavy workloads
    // Readers: shared_lock (multiple concurrent)
    // Writers: unique_lock (exclusive)
    mutable std::shared_mutex mutex_;
    
public:
    // ========== STRING COMMANDS ==========
    bool set(const std::string& key, const std::string& value, int ttl = 0);
    std::optional<std::string> get(const std::string& key) const;
    
    // ========== LIST COMMANDS ==========
    size_t lpush(const std::string& key, const std::vector<std::string>& values);
    size_t rpush(const std::string& key, const std::vector<std::string>& values);
    std::optional<std::string> lpop(const std::string& key);
    std::optional<std::string> rpop(const std::string& key);
    std::vector<std::string> lrange(const std::string& key, int start, int stop) const;
    size_t llen(const std::string& key) const;
    
    // ========== SET COMMANDS ==========
    size_t sadd(const std::string& key, const std::vector<std::string>& members);
    size_t srem(const std::string& key, const std::vector<std::string>& members);
    bool sismember(const std::string& key, const std::string& member) const;
    std::vector<std::string> smembers(const std::string& key) const;
    size_t scard(const std::string& key) const;
    
    // ========== HASH COMMANDS ==========
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    std::optional<std::string> hget(const std::string& key, const std::string& field) const;
    size_t hdel(const std::string& key, const std::vector<std::string>& fields);
    bool hexists(const std::string& key, const std::string& field) const;
    std::unordered_map<std::string, std::string> hgetall(const std::string& key) const;
    size_t hlen(const std::string& key) const;
    
    // ========== GENERAL COMMANDS ==========
    bool del(const std::string& key);
    bool exists(const std::string& key) const;
    std::optional<ValueType> type(const std::string& key) const;
    bool expire(const std::string& key, int seconds);
    int ttl(const std::string& key) const;
    std::vector<std::string> keys() const;
    size_t size() const;
    void clear();
    
    // Access underlying store (for persistence)
    KeyValueStore& getStore() { return store_; }
    const KeyValueStore& getStore() const { return store_; }
};

// ============================================================================
// CONCURRENCY CONCEPTS:
// ============================================================================
//
// 1. std::shared_mutex (C++17):
//    - Read-write lock
//    - Multiple readers can hold lock simultaneously
//    - Only one writer can hold lock (exclusive)
//    - Perfect for read-heavy workloads (like caches)
//
//    Performance comparison:
//    - std::mutex: All operations exclusive (slower for reads)
//    - std::shared_mutex: Reads concurrent (faster)
//
//    Example:
//      shared_mutex m;
//      
//      // Reader (can be multiple)
//      shared_lock<shared_mutex> lock(m);
//      
//      // Writer (exclusive)
//      unique_lock<shared_mutex> lock(m);
//
// 2. Why mutable mutex in const methods?
//    - const methods promise not to modify *logical state*
//    - Mutex is *implementation detail* for synchronization
//    - mutable allows locking in const methods (like get, exists)
//
// 3. Lock granularity:
//    - Fine-grained: Lock individual keys (complex but faster)
//    - Coarse-grained: Lock entire store (simple but slower)
//    - We use coarse-grained for simplicity
//    - Real Redis uses single thread + event loop (no locks!)
//
// 4. Deadlock prevention:
//    - Acquire locks in consistent order
//    - Use RAII (lock_guard, unique_lock)
//    - Keep critical sections short
//
// ============================================================================

#endif // THREADSAFESTORE_H