#include "Object.hpp"


namespace types {
    class Number final: public Object {
        private:
            double value;
        public:
            Number(int number);
            Number(double number);
            Number(Number& number);
            bool Equals(Object&);	
            bool Equals(Object& a, Object& b);	
            bool Equals(Number&);	
            bool Equals(Number& a, Number& b);	
            void Finalize();	
            size_t GetHashCode();	
            void GetType();	
            bool ReferenceEquals(Object& a, Object& b);	
            bool ReferenceEquals(Number& a, Number& b);	
            std::string ToString();
    };
}