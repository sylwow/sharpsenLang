#include "Object.hpp"


namespace types {
    class String final: public Object {
        private:
            std::string value;
        public:
            String(std::string& string);
            String(String& string);
            bool Equals(Object&);	
            bool Equals(Object& a, Object& b);	
            bool Equals(String&);	
            bool Equals(String& a, String& b);	
            void Finalize();	
            size_t GetHashCode();	
            void GetType();	
            bool ReferenceEquals(Object& a, Object& b);	
            bool ReferenceEquals(String& a, String& b);	
            std::string ToString();
    };
}