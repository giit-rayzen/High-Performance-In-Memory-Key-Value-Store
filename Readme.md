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