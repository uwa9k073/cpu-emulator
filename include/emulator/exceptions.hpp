#pragma once

#include <stdexcept>
namespace cpu_emulator::core {
class Error : public std::runtime_error {
    using runtime_error::runtime_error;
};

class RuntimeError : public Error {
    using Error::Error;
};

class ParsingError : public RuntimeError {
    using RuntimeError::RuntimeError;
};
}  // namespace cpu_emulator::core
