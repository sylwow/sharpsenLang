#include "Number.hpp"

namespace types {
    Number::Number(int number) {
        value = number;
    }

    Number::Number(double number) {
        value = number;
    }

    Number::Number(Number& number) {
        value = number.value;
    }

    bool Number::Equals(Object& object) {
        auto* number = dynamic_cast<Number*>(&object);
        if (number != nullptr) {
            return Equals(*number);
        } else {
            return false;
        }
    }

    bool Number::Equals(Object& a, Object& b) {
        auto* numberA = dynamic_cast<Number*>(&a);
        auto* numberB = dynamic_cast<Number*>(&b);
        if (numberA != nullptr && numberB != nullptr) {
            return Equals(*numberA, *numberB);
        } else {
            return false;
        }
    }	

    bool Number::Equals(Number& object) {
        return value == object.value;
    }

    bool Number::Equals(Number& a, Number& b) {
        return a.value == b.value;
    }	

    void Number::Finalize() {}	
    
    size_t Number::GetHashCode() { return value; }	

    void Number::GetType() {}
    
    bool Number::ReferenceEquals(Object& a, Object& b) {
        return Equals(a, b);
    }	

    bool Number::ReferenceEquals(Number& a, Number& b) {
        return Equals(a, b);
    }

    std::string Number::ToString() { return std::to_string(value); }
}