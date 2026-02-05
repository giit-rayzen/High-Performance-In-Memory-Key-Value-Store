#include "../include/ThreadSafeStore.h"

// ========== STRING COMMANDS ==========

bool ThreadSafeStore::set(const std::string& key, const std::string& value, int ttl) {
    // Write operation - exclusive lock
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return store_.set(key, value, ttl);
}

std::optional<std::string> ThreadSafeStore::get(const std::string& key) const {
    // Read operation - shared lock (multiple readers allowed)
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.get(key);
}

// ========== LIST COMMANDS ==========

size_t ThreadSafeStore::lpush(const std::string& key, const std::vector<std::string>& values) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return store_.lpush(key, values);
}

size_t ThreadSafeStore::rpush(const std::string& key, const std::vector<std::string>& values) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return store_.rpush(key, values);
}

std::optional<std::string> ThreadSafeStore::lpop(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return store_.lpop(key);
}

std::optional<std::string> ThreadSafeStore::rpop(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return store_.rpop(key);
}

std::vector<std::string> ThreadSafeStore::lrange(const std::string& key, int start, int stop) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.lrange(key, start, stop);
}

size_t ThreadSafeStore::llen(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.llen(key);
}

// ========== SET COMMANDS ==========

size_t ThreadSafeStore::sadd(const std::string& key, const std::vector<std::string>& members) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return store_.sadd(key, members);
}

size_t ThreadSafeStore::srem(const std::string& key, const std::vector<std::string>& members) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return store_.srem(key, members);
}

bool ThreadSafeStore::sismember(const std::string& key, const std::string& member) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.sismember(key, member);
}

std::vector<std::string> ThreadSafeStore::smembers(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.smembers(key);
}

size_t ThreadSafeStore::scard(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.scard(key);
}

// ========== HASH COMMANDS ==========

bool ThreadSafeStore::hset(const std::string& key, const std::string& field, const std::string& value) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return store_.hset(key, field, value);
}

std::optional<std::string> ThreadSafeStore::hget(const std::string& key, const std::string& field) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.hget(key, field);
}

size_t ThreadSafeStore::hdel(const std::string& key, const std::vector<std::string>& fields) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return store_.hdel(key, fields);
}

bool ThreadSafeStore::hexists(const std::string& key, const std::string& field) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.hexists(key, field);
}

std::unordered_map<std::string, std::string> ThreadSafeStore::hgetall(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.hgetall(key);
}

size_t ThreadSafeStore::hlen(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.hlen(key);
}

// ========== GENERAL COMMANDS ==========

bool ThreadSafeStore::del(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return store_.del(key);
}

bool ThreadSafeStore::exists(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.exists(key);
}

std::optional<ValueType> ThreadSafeStore::type(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.type(key);
}

bool ThreadSafeStore::expire(const std::string& key, int seconds) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return store_.expire(key, seconds);
}

int ThreadSafeStore::ttl(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.ttl(key);
}

std::vector<std::string> ThreadSafeStore::keys() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.keys();
}

size_t ThreadSafeStore::size() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.size();
}

void ThreadSafeStore::clear() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    store_.clear();
}

// ============================================================================
// PERFORMANCE NOTES:
// ============================================================================
//
// Why shared_mutex over regular mutex?
//
// Scenario: Web cache with 90% reads, 10% writes
//
// With std::mutex:
//   - All operations block each other
//   - 100 concurrent readers = 100x slower
//   - Read throughput: ~1000 ops/sec
//
// With std::shared_mutex:
//   - Readers don't block readers
//   - 100 concurrent readers = almost same speed as 1
//   - Read throughput: ~100,000 ops/sec (100x better!)
//   - Write throughput: same as mutex
//
// Trade-offs:
// + Much faster for read-heavy workloads (most caches)
// + Natural fit for Redis (mostly GET operations)
// - Slightly more overhead per lock (not noticeable)
// - More complex implementation
//
// When to use regular mutex instead?
// - Write-heavy workloads (50%+ writes)
// - Very short critical sections (< 10 instructions)
// - Simplicity over performance
//
// Real Redis approach:
// - Single-threaded event loop (no locks!)
// - I/O multiplexing (epoll/kqueue)
// - Why? Lock contention is expensive, single thread is often faster
// - Use threads only for slow I/O (disk, network)
//
// Our approach (shared_mutex):
// - Good balance for learning
// - Shows proper concurrency techniques
// - Performs well for realistic workloads
//
// ============================================================================