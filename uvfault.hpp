#pragma once

#include <cstdint>
#include <asmjit/core.h>
#include <asmjit/x86.h>

using uvfault_fn = int64_t (*)(int64_t seed, uint32_t iters);

void uvfault_generate_imul(asmjit::x86::Assembler &as, int depth, int pauseDepth);
void uvfault_generate_imul_imm(asmjit::x86::Assembler &as, int depth, int pauseDepth);
void uvfault_generate_aesenc(asmjit::x86::Assembler &as, int depth, int pauseDepth);

class AsmException : std::exception {
public:
    AsmException(int err) : _err(err) {
    }
    const char *what() const noexcept override {
        return asmjit::DebugUtils::errorAsString(_err);
    }

private:
    int _err;
};
