#include "Object.hpp"
#include <filesystem>
#include <functional>

namespace types {
    bool Object::Equals(Object& other) {
        return this == &other;
    }	

    bool Object::Equals(Object& a, Object& b) {
        return ReferenceEquals(a, b);
    }

    void Object::Finalize() {}

    size_t Object::GetHashCode() {
        return 1;
    }

    void Object::GetType() {}

    bool Object::ReferenceEquals(Object& a, Object& b) {
        return &a == &b;
    }

    std::string Object::ToString() {
        std::ostringstream oss;
        oss << (void*)this;
        return std::string(oss.str());
    }
    
}