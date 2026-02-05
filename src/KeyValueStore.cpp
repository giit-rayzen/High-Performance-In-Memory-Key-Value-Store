#include "../include/KeyValueStore.h"

// ========== STRING COMMANDS ==========

bool KeyValueStore::set(const std::string& key, const std::string& value, int ttl) {
    return storage_.set(key, value, ttl);
}

std::optional<std::string> KeyValueStore::get(const std::string& key) const {
    // const_cast needed because storage methods check expiration (modify state)
    // Alternative: make storage_ mutable (cleaner)
    return const_cast<StorageEngine&>(storage_).get(key);
}

// ========== LIST COMMANDS ==========

size_t KeyValueStore::lpush(const std::string& key, const std::vector<std::string>& values) {
    return storage_.lpush(key, values);
}

size_t KeyValueStore::rpush(const std::string& key, const std::vector<std::string>& values) {
    return storage_.rpush(key, values);
}

std::optional<std::string> KeyValueStore::lpop(const std::string& key) {
    return storage_.lpop(key);
}

std::optional<std::string> KeyValueStore::rpop(const std::string& key) {
    return storage_.rpop(key);
}

std::vector<std::string> KeyValueStore::lrange(const std::string& key, int start, int stop) const {
    return const_cast<StorageEngine&>(storage_).lrange(key, start, stop);
}

size_t KeyValueStore::llen(const std::string& key) const {
    return const_cast<StorageEngine&>(storage_).llen(key);
}

// ========== SET COMMANDS ==========

size_t KeyValueStore::sadd(const std::string& key, const std::vector<std::string>& members) {
    return storage_.sadd(key, members);
}

size_t KeyValueStore::srem(const std::string& key, const std::vector<std::string>& members) {
    return storage_.srem(key, members);
}

bool KeyValueStore::sismember(const std::string& key, const std::string& member) const {
    return const_cast<StorageEngine&>(storage_).sismember(key, member);
}

std::vector<std::string> KeyValueStore::smembers(const std::string& key) const {
    return const_cast<StorageEngine&>(storage_).smembers(key);
}

size_t KeyValueStore::scard(const std::string& key) const {
    return const_cast<StorageEngine&>(storage_).scard(key);
}

// ========== HASH COMMANDS ==========

bool KeyValueStore::hset(const std::string& key, const std::string& field, const std::string& value) {
    return storage_.hset(key, field, value);
}

std::optional<std::string> KeyValueStore::hget(const std::string& key, const std::string& field) const {
    return const_cast<StorageEngine&>(storage_).hget(key, field);
}

size_t KeyValueStore::hdel(const std::string& key, const std::vector<std::string>& fields) {
    return storage_.hdel(key, fields);
}

bool KeyValueStore::hexists(const std::string& key, const std::string& field) const {
    return const_cast<StorageEngine&>(storage_).hexists(key, field);
}

std::unordered_map<std::string, std::string> KeyValueStore::hgetall(const std::string& key) const {
    return const_cast<StorageEngine&>(storage_).hgetall(key);
}

size_t KeyValueStore::hlen(const std::string& key) const {
    return const_cast<StorageEngine&>(storage_).hlen(key);
}

// ========== GENERAL COMMANDS ==========

bool KeyValueStore::del(const std::string& key) {
    return storage_.remove(key);
}

bool KeyValueStore::exists(const std::string& key) const {
    return const_cast<StorageEngine&>(storage_).exists(key);
}

std::optional<ValueType> KeyValueStore::type(const std::string& key) const {
    return const_cast<StorageEngine&>(storage_).type(key);
}

bool KeyValueStore::expire(const std::string& key, int seconds) {
    return storage_.expire(key, seconds);
}

int KeyValueStore::ttl(const std::string& key) const {
    return const_cast<StorageEngine&>(storage_).ttl(key);
}

std::vector<std::string> KeyValueStore::keys() const {
    return storage_.keys();
}

size_t KeyValueStore::size() const {
    return storage_.size();
}

void KeyValueStore::clear() {
    storage_.clear();
}

// ============================================================================
// DESIGN PATTERN: Delegation / Facade Pattern
// ============================================================================
//
// Why this design?
//
// 1. Separation of Concerns:
//    - KeyValueStore: Command routing, API surface
//    - StorageEngine: Actual data management
//    - Easy to add layers: ThreadSafeStore, PersistenceManager
//
// 2. Single Responsibility:
//    - Each class has ONE reason to change
//    - Storage logic changes? Only StorageEngine affected
//    - API changes? Only KeyValueStore affected
//
// 3. Testability:
//    - Can test StorageEngine independently
//    - Can mock StorageEngine for KeyValueStore tests
//
// 4. Flexibility:
//    - Easy to swap storage backend (memory → disk → distributed)
//    - Easy to add caching layer
//    - Easy to add metrics/logging
//
// 5. Open/Closed Principle (SOLID):
//    - Open for extension (add new commands)
//    - Closed for modification (don't change existing code)
//
// Alternative designs (and why we didn't use them):
//
// A. Single monolithic class:
//    class Redis { ... all logic ... }
//    - Hard to test
//    - Hard to maintain
//    - Violates SRP
//
// B. Inheritance:
//    class KeyValueStore : public StorageEngine { ... }
//    - Tight coupling
//    - Can't easily swap implementation
//    - Violates "favor composition over inheritance"
//
// C. Static methods:
//    class Redis { static void set(...); }
//    - Hard to test (can't mock)
//    - Global state
//    - Not thread-safe
//
// Our design (delegation) is clean, testable, and maintainable.
//
// ============================================================================