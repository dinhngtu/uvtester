#include <stdexcept>
#include <random>
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

static void step_imul_imm(Assembler &as, int depth, Gpq res, std::mt19937 &gen) {
    std::uniform_int_distribution<int32_t> distr(INT32_MIN, INT32_MAX);
    for (int i = 0; i < depth; i++) {
        int32_t imm;
        do {
            imm = distr(gen);
        } while (imm >= -1 && imm <= 1);
        as.imul(res, res, imm);
    }
}

static void step_aesenc(Assembler &as, int depth, Xmm res) {
    for (int i = 0; i < depth; i++)
        as.aesenc(res, res);
}

void uvfault_generate_imul(Assembler &as, int mulDepth, int pauseDepth) {
    if (mulDepth < 1)
        throw std::invalid_argument("depth");
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

void uvfault_generate_imul_imm(Assembler &as, int depth, int pauseDepth) {
    if (depth < 1)
        throw std::invalid_argument("depth");
    if (pauseDepth < 0)
        throw std::invalid_argument("pauseDepth");

#if !_WIN32
    // use rcx as seed, edx as iter count
    as.mov(rcx, rdi);
    as.mov(edx, esi);
#endif

    // rax, rcx, rdx, r8, r9, r10, r11 are always volatile registers

    as.xor_(rax, rax);

    std::random_device rd;
    std::mt19937 gen;
    auto immseed = rd();

    auto loop = as.newLabel();
    as.bind(loop);
    as.mov(r8, rcx);
    gen.seed(immseed);
    step_imul_imm(as, depth, r8, gen);
    as.mov(r9, rcx);
    gen.seed(immseed);
    step_imul_imm(as, depth, r9, gen);
    as.xor_(r8, r9);
    as.or_(rax, r8);
    as.dec(edx);
    as.jnz(loop);
    step_pause(as, pauseDepth);
    as.ret();
}

void uvfault_generate_aesenc(Assembler &as, int depth, int pauseDepth) {
    if (depth < 1)
        throw std::invalid_argument("depth");
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
    as.movq(xmm0, rcx);
    as.punpcklqdq(xmm0, xmm0);
    step_aesenc(as, depth, xmm0);
    as.movq(xmm1, rcx);
    as.punpcklqdq(xmm1, xmm1);
    step_aesenc(as, depth, xmm1);
    as.pxor(xmm0, xmm1);
    as.movq(r8, xmm0);
    as.pextrq(r9, xmm0, 1);
    as.or_(rax, r8);
    as.or_(rax, r9);
    as.dec(edx);
    as.jnz(loop);
    step_pause(as, pauseDepth);
    as.ret();
}
