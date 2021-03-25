#pragma once
#include <iostream>
#include <exception>

namespace anal {

    class SyntaxError: public std::exception {
        public:
            explicit SyntaxError(const char* message)
                : msg_(message) {}

            explicit SyntaxError(const std::string& message)
                : msg_(message) {}

            virtual ~SyntaxError() noexcept {}

            virtual const char* what() const noexcept {
                return msg_.c_str();
            }

        protected:
            /** Error message.
             */
            std::string msg_;
    };
}