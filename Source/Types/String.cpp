#include "String.hpp"

namespace types {
    String::String(std::string& string) {
        value = string;
    }

    String::String(String& string) {
        value = string.value;
    }

    bool String::Equals(Object& object) {
        auto* string = dynamic_cast<String*>(&object);
        if (string != nullptr) {
            return Equals(*string);
        } else {
            return false;
        }
    }

    bool String::Equals(Object& a, Object& b) {
        auto* StringA = dynamic_cast<String*>(&a);
        auto* StringB = dynamic_cast<String*>(&b);
        if (StringA != nullptr && StringB != nullptr) {
            return Equals(*StringA, *StringB);
        } else {
            return false;
        }
    }	

    bool String::Equals(String& object) {
        return value == object.value;
    }

    bool String::Equals(String& a, String& b) {
        return a.value == b.value;
    }	

    void String::Finalize() {}	
    
    size_t String::GetHashCode() { return 1; }	

    void String::GetType() {}
    
    bool String::ReferenceEquals(Object& a, Object& b) {
        return Equals(a, b);
    }	

    bool String::ReferenceEquals(String& a, String& b) {
        return Equals(a, b);
    }

    std::string String::ToString() { return value; }
}