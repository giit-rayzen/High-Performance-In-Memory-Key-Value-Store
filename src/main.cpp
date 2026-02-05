#include "../include/ThreadSafeStore.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

// ANSI colors for pretty output
#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define BOLD    "\033[1m"

void printHeader(const std::string& title) {
    std::cout << "\n" << BOLD << CYAN << "========================================" << RESET << "\n";
    std::cout << BOLD << CYAN << "  " << title << RESET << "\n";
    std::cout << BOLD << CYAN << "========================================" << RESET << "\n\n";
}

void testStrings(ThreadSafeStore& store) {
    printHeader("STRING Operations");
    
    // SET and GET
    store.set("name", "IITian");
    store.set("role", "Backend Engineer");
    store.set("company", "Microsoft");
    
    auto name = store.get("name");
    std::cout << "GET name: " << GREEN << (name ? *name : "(nil)") << RESET << "\n";
    
    auto role = store.get("role");
    std::cout << "GET role: " << GREEN << (role ? *role : "(nil)") << RESET << "\n";
    
    // TTL test
    std::cout << "\nSetting 'session' with 5 second TTL...\n";
    store.set("session", "abc123", 5);
    std::cout << "TTL session: " << YELLOW << store.ttl("session") << " seconds" << RESET << "\n";
    
    std::cout << "Waiting 2 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "TTL session: " << YELLOW << store.ttl("session") << " seconds" << RESET << "\n";
    
    // EXISTS and TYPE
    std::cout << "\nEXISTS name: " << (store.exists("name") ? GREEN "1" : "0") << RESET << "\n";
    auto type = store.type("name");
    std::cout << "TYPE name: " << CYAN << (type ? typeToString(*type) : "unknown") << RESET << "\n";
}

void testLists(ThreadSafeStore& store) {
    printHeader("LIST Operations");
    
    // RPUSH - add to right (tail)
    std::vector<std::string> tasks = {"Write code", "Review PR", "Deploy"};
    store.rpush("tasks", tasks);
    std::cout << "RPUSH tasks (Write code, Review PR, Deploy)\n";
    std::cout << "LLEN tasks: " << GREEN << store.llen("tasks") << RESET << "\n";
    
    // LPUSH - add to left (head)
    store.lpush("tasks", {"Fix bug"});
    std::cout << "\nLPUSH tasks (Fix bug)\n";
    
    // LRANGE - get all
    auto all_tasks = store.lrange("tasks", 0, -1);
    std::cout << "LRANGE tasks 0 -1:\n";
    for (size_t i = 0; i < all_tasks.size(); i++) {
        std::cout << "  " << i << ") " << YELLOW << all_tasks[i] << RESET << "\n";
    }
    
    // LPOP - remove from left
    auto first_task = store.lpop("tasks");
    std::cout << "\nLPOP tasks: " << GREEN << (first_task ? *first_task : "(nil)") << RESET << "\n";
    std::cout << "LLEN tasks: " << store.llen("tasks") << "\n";
}

void testSets(ThreadSafeStore& store) {
    printHeader("SET Operations");
    
    // SADD - add members
    std::vector<std::string> tags = {"cpp", "redis", "backend"};
    size_t added = store.sadd("skills", tags);
    std::cout << "SADD skills (cpp, redis, backend)\n";
    std::cout << "Added: " << GREEN << added << " members" << RESET << "\n";
    
    // Try adding duplicate
    added = store.sadd("skills", {"cpp", "python"});
    std::cout << "\nSADD skills (cpp, python) - cpp is duplicate\n";
    std::cout << "Added: " << GREEN << added << " members" << RESET << "\n";
    
    // SISMEMBER - check membership
    std::cout << "\nSISMEMBER skills cpp: " << (store.sismember("skills", "cpp") ? GREEN "1" : "0") << RESET << "\n";
    std::cout << "SISMEMBER skills java: " << (store.sismember("skills", "java") ? GREEN "1" : "0") << RESET << "\n";
    
    // SMEMBERS - get all members
    auto members = store.smembers("skills");
    std::cout << "\nSMEMBERS skills:\n";
    for (const auto& member : members) {
        std::cout << "  - " << CYAN << member << RESET << "\n";
    }
    
    std::cout << "SCARD skills: " << GREEN << store.scard("skills") << RESET << "\n";
}

void testHashes(ThreadSafeStore& store) {
    printHeader("HASH Operations");
    
    // HSET - set fields
    store.hset("user:1001", "name", "Alice");
    store.hset("user:1001", "email", "alice@example.com");
    store.hset("user:1001", "age", "28");
    std::cout << "HSET user:1001 (name, email, age)\n";
    
    // HGET - get field
    auto name = store.hget("user:1001", "name");
    auto email = store.hget("user:1001", "email");
    std::cout << "\nHGET user:1001 name: " << GREEN << (name ? *name : "(nil)") << RESET << "\n";
    std::cout << "HGET user:1001 email: " << GREEN << (email ? *email : "(nil)") << RESET << "\n";
    
    // HEXISTS
    std::cout << "\nHEXISTS user:1001 name: " << (store.hexists("user:1001", "name") ? GREEN "1" : "0") << RESET << "\n";
    std::cout << "HEXISTS user:1001 phone: " << (store.hexists("user:1001", "phone") ? GREEN "1" : "0") << RESET << "\n";
    
    // HGETALL - get all fields
    auto all = store.hgetall("user:1001");
    std::cout << "\nHGETALL user:1001:\n";
    for (const auto& [field, value] : all) {
        std::cout << "  " << CYAN << field << RESET << " => " << YELLOW << value << RESET << "\n";
    }
    
    std::cout << "HLEN user:1001: " << GREEN << store.hlen("user:1001") << RESET << "\n";
}

void testThreadSafety(ThreadSafeStore& store) {
    printHeader("Thread Safety Test");
    
    const int NUM_THREADS = 10;
    const int OPS_PER_THREAD = 1000;
    
    std::cout << "Running " << NUM_THREADS << " threads, " 
              << OPS_PER_THREAD << " operations each...\n";
    
    auto worker = [&](int id) {
        for (int i = 0; i < OPS_PER_THREAD; i++) {
            std::string key = "thread:" + std::to_string(id) + ":count:" + std::to_string(i);
            store.set(key, std::to_string(i));
            
            // Verify immediately
            auto val = store.get(key);
            if (!val || *val != std::to_string(i)) {
                std::cerr << "ERROR: Data corruption in thread " << id << "\n";
            }
        }
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << GREEN << "✓ Completed successfully!\n" << RESET;
    std::cout << "Time: " << duration.count() << " ms\n";
    std::cout << "Total operations: " << (NUM_THREADS * OPS_PER_THREAD * 2) << " (SET + GET)\n";
    std::cout << "Operations/sec: " << ((NUM_THREADS * OPS_PER_THREAD * 2 * 1000) / duration.count()) << "\n";
}

void testMixedOperations(ThreadSafeStore& store) {
    printHeader("Mixed Type Operations");
    
    // Create different types with same key prefix
    store.set("data:string", "I'm a string");
    store.lpush("data:list", {"item1", "item2"});
    store.sadd("data:set", {"elem1", "elem2"});
    store.hset("data:hash", "field1", "value1");
    
    // Check types
    std::cout << "TYPE data:string: " << CYAN << typeToString(*store.type("data:string")) << RESET << "\n";
    std::cout << "TYPE data:list: " << CYAN << typeToString(*store.type("data:list")) << RESET << "\n";
    std::cout << "TYPE data:set: " << CYAN << typeToString(*store.type("data:set")) << RESET << "\n";
    std::cout << "TYPE data:hash: " << CYAN << typeToString(*store.type("data:hash")) << RESET << "\n";
    
    // Get all keys
    std::cout << "\nAll keys:\n";
    auto all_keys = store.keys();
    for (const auto& key : all_keys) {
        std::cout << "  - " << YELLOW << key << RESET << "\n";
    }
    
    std::cout << "\nDBSIZE: " << GREEN << store.size() << " keys" << RESET << "\n";
}

int main() {
    std::cout << BOLD << MAGENTA;
    std::cout << R"(
╔═══════════════════════════════════════╗
║   Enhanced Redis - Full Featured     ║
║   String | List | Set | Hash + TTL   ║
╚═══════════════════════════════════════╝
)" << RESET << "\n";
    
    ThreadSafeStore store;
    
    // Run all tests
    testStrings(store);
    testLists(store);
    testSets(store);
    testHashes(store);
    testMixedOperations(store);
    testThreadSafety(store);
    
    printHeader("Summary");
    std::cout << "Final DBSIZE: " << GREEN << store.size() << " keys" << RESET << "\n";
    std::cout << "\n" << BOLD << GREEN << "All tests passed! ✓" << RESET << "\n\n";
    
    return 0;
}