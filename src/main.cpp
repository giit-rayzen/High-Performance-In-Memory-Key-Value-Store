#include <iostream>
#include "KeyValueStore.h"

int main() {
    KeyValueStore kv;

    kv.set("name", "IITian");
    kv.set("role", "Backend Engineer");

    auto val = kv.get("name");
    if (val) {
        std::cout << "name: " << *val << "\n";
    }

    std::cout << "exists(role): " << kv.exists("role") << "\n";
    std::cout << "size: " << kv.size() << "\n";

    kv.remove("role");
    std::cout << "size after delete: " << kv.size() << "\n";

    kv.clear();
    std::cout << "size after clear: " << kv.size() << "\n";

    return 0;
}
