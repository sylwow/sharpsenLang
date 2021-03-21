#pragma once
#include <iostream>
#include <filesystem>

namespace types {
    class Object {
        public:
            virtual bool Equals(Object&);	
            virtual bool Equals(Object& a, Object& b);	
            virtual void Finalize();	
            virtual size_t GetHashCode();	
            virtual void GetType();	
            virtual bool ReferenceEquals(Object& a, Object& b);	
            virtual std::string ToString();
    };
}