#include <stdexcept>
#include "uvfault.hpp"

using namespace asmjit::x86;

static void step_pause(Assembler &as, int depth) {
    for (int i = 0; i < depth; i++)
        as.pause();
}

static void step_imul(Assembler &as, int depth, Gpq res) {
    for (int i = 0; i < depth; i++)
        as.imul(res, res);
}

void uvfault_generate_imul(Assembler &as, int mulDepth, int pauseDepth) {
    if (mulDepth < 1)
        throw std::invalid_argument("mulDepth");
    if (pauseDepth < 0)
        throw std::invalid_argument("pauseDepth");

#if !_WIN32
    // use rcx as seed, edx as iter count
    as.mov(rcx, rdi);
    as.mov(edx, esi);
#endif

    // rax, rcx, rdx, r8, r9, r10, r11 are always volatile registers

    as.xor_(rax, rax);

    auto loop = as.newLabel();
    as.bind(loop);
    as.mov(r8, rcx);
    step_imul(as, mulDepth, r8);
    as.mov(r9, rcx);
    step_imul(as, mulDepth, r9);
    as.xor_(r8, r9);
    as.or_(rax, r8);
    as.dec(edx);
    as.jnz(loop);
    step_pause(as, pauseDepth);
    as.ret();
}
