#include <stdexcept>
#include "uvfault.hpp"

using namespace asmjit::x86;

static void step_pause(Assembler &as, int depth) {
    for (int i = 0; i < depth; i++)
        as.pause();
}

static void step_imul(Assembler &as, int depth, Gpq res, Gpq tmpa, Gpq tmpb) {
    switch (depth) {
    case 1:
        as.imul(res, res);
        break;
    case 2:
        as.lea(tmpa, ptr(res, res));
        as.imul(tmpa, res);
        as.imul(res, tmpa);
        break;
    case 3:
        as.lea(tmpa, ptr(res, res));
        as.imul(tmpa, res);
        as.imul(tmpa, tmpa);
        as.imul(res, tmpa);
        break;
    case 4:
        as.lea(tmpa, ptr(res, res));
        as.mov(tmpb, res);
        as.imul(tmpa, res);
        as.imul(tmpb, res);
        as.imul(res, tmpa);
        as.imul(res, tmpb);
        break;
    }
}

void uvfault_generate_imul(Assembler &as, int mulDepth, int pauseDepth) {
    if (mulDepth < 1 || mulDepth > 4)
        throw std::invalid_argument("mulDepth");
    if (pauseDepth < 0 || pauseDepth > 8)
        throw std::invalid_argument("pauseDepth");

#if !_WIN32
    // use rcx as seed, edx as iter count
    as.mov(rcx, rdi);
    as.mov(edx, esi);
#endif

    // rax, rcx, rdx, r8, r9, r10, r11 are always volatile registers

    auto tmpa = r10;
    auto tmpb = r11;
    as.xor_(rax, rax);

    auto loop = as.newLabel();
    as.bind(loop);
    as.mov(r8, rcx);
    step_imul(as, mulDepth, r8, tmpa, tmpb);
    as.mov(r9, rcx);
    step_imul(as, mulDepth, r9, tmpa, tmpb);
    as.xor_(r8, r9);
    as.or_(rax, r8);
    as.dec(edx);
    as.jnz(loop);
    step_pause(as, pauseDepth);
    as.ret();
}
