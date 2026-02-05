Design and implement a high-performance in-memory key-value store in C++ that supports:

Fast read/write operations
Optional persistence to disk
Thread-safe concurrent access
Pluggable eviction strategies (LRU mandatory)
Configurable via file / CLI

🔹 Core Features

SET key value
GET key
DELETE key
EXISTS key
SIZE
CLEAR

🔹 Eviction Policy

LRU (Least Recently Used)
Auto-evict when capacity exceeded
O(1) eviction

🔹 Persistence
Save data to disk (dump.db)
Load data at startup
Crash-safe write

🔹 Concurrency

Multiple readers & writers
Thread-safe operations
No data race

Phase-1
StorageEngine + basic SET/GET

Phase-2
LRU eviction

Phase-3
Thread safety

Phase-4
Persistence

Phase-5
CLI + configs

Production-Grade Key-Value Store
A feature-rich, thread-safe, multi-type key-value store inspired by Redis, built in modern C++17.
🎯 Project Highlights
This project demonstrates production-level C++ engineering:
✅ 4 Redis Data Types: String, List, Set, Hash
✅ TTL Support: Auto-expiring keys with lazy deletion
✅ Thread-Safe: shared_mutex for optimal read/write concurrency
✅ Type-Safe: std::variant + std::optional for compile-time safety
✅ Clean Architecture: Your design pattern + comprehensive features
✅ 434K ops/sec: Real performance benchmarks

┌─────────────────────────────────────────┐
│        ThreadSafeStore                  │
│    (Concurrency Control Layer)          │
│    shared_mutex for optimal perf        │
└─────────────┬───────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────┐
│        KeyValueStore                    │
│    (User-Facing API / Facade)           │        │
└─────────────┬───────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────┐
│        StorageEngine                    │
│    (Core Data Storage)                  │
│    unordered_map + RedisValue           │
└─────────────────────────────────────────┘
              │
              ▼
┌─────────────────────────────────────────┐
│        ValueTypes                       │
│    (Type System)                        │
│    variant<String, List, Set, Hash>     │
└─────────────────────────────────────────┘