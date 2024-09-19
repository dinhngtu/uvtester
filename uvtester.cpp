#include <cstdio>
#include <cinttypes>
#include <chrono>
#include <thread>
#include <random>
#include <stdexcept>
#include <cxxopts.hpp>
#include "uvfault.hpp"

enum class InstructionType { Imul, Max };

static const char *instruction_types[static_cast<int>(InstructionType::Max)] = {"imul"};

void parse_value([[maybe_unused]] const std::string &text, InstructionType &value) {
    value = InstructionType::Imul;
}

struct Args {
    InstructionType method;
    int depth;
    uint32_t iters;
    int pausedepth;
    long long passes;
    int sleep;
    int measure;
};

static cxxopts::Options make_options() {
    cxxopts::Options opt{"uvtester"};
    auto g = opt.add_options();
    g("method", "instruction type", cxxopts::value<InstructionType>()->default_value("imul"));
    g("depth", "faulting instruction depth per half iteration", cxxopts::value<int>()->default_value("4"));
    g("iters", "iteration count per check", cxxopts::value<uint32_t>()->default_value("10"));
    g("pausedepth", "pause depth per check", cxxopts::value<int>()->default_value("0"));
    g("passes", "number of iterated passes per sleep", cxxopts::value<long long>()->default_value("25000"));
    g("sleep", "sleep time in milliseconds", cxxopts::value<int>()->default_value("1"));
    g("measure", "measure time of main loops", cxxopts::value<int>()->default_value("0"));
    return opt;
}

void doit(const Args &args) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int64_t> distr(INT64_MIN, INT64_MAX);

    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    code.init(rt.environment(), rt.cpuFeatures());
    asmjit::x86::Assembler as(&code);
    uvfault_generate_imul(as, args.depth, args.pausedepth);
    uvfault_fn fn;
    auto err = rt.add(&fn, &code);
    if (err)
        throw AsmException(err);

    long long avg;
    int loopit = 0;
    while (1) {
        std::chrono::high_resolution_clock::time_point begin;
        if (args.measure) {
            begin = std::chrono::high_resolution_clock::now();
            if (loopit % args.measure == 0)
                avg = 0;
        }
        for (long long i = 0; i < args.passes; i++) {
            int64_t seed;
            do {
                seed = distr(gen);
            } while (seed >= -1 && seed <= 1);
            auto res = fn(seed, args.iters);
            if (res)
                printf("bad result %" PRId64 "x\n", res);
        }
        if (args.measure) {
            auto tm = std::chrono::high_resolution_clock::now() - begin;
            auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(tm).count();
            avg += usecs;
            if (loopit % args.measure == args.measure - 1)
                printf("total %lld us\n", avg);
            loopit++;
        }
        if (args.sleep)
            std::this_thread::sleep_for(std::chrono::milliseconds(args.sleep));
    }
}

int main(int argc, char **argv) {
    Args args{};

    auto opts = make_options();
    cxxopts::ParseResult argm;
    try {
        argm = opts.parse(argc, argv);
        args.method = argm["method"].as<InstructionType>();
        args.depth = argm["depth"].as<int>();
        args.iters = argm["iters"].as<uint32_t>();
        args.pausedepth = argm["pausedepth"].as<int>();
        args.passes = argm["passes"].as<long long>();
        if (args.passes < 0)
            throw std::invalid_argument("passes");
        args.sleep = argm["sleep"].as<int>();
        if (args.sleep < 0)
            throw std::invalid_argument("sleep");
        args.measure = argm["measure"].as<int>();
        if (args.measure < 0)
            throw std::invalid_argument("sleep");
    } catch (const std::exception &ex) {
        printf("%s\n", ex.what());
        auto h = opts.help();
        printf("%s\n", h.c_str());
        return 1;
    }
    printf("method:\t\t%s\n", instruction_types[static_cast<int>(args.method)]);
    printf("depth:\t\t%d\n", args.depth);
    printf("iters:\t\t%" PRIu32 "\n", args.iters);
    printf("pausedepth:\t%d\n", args.pausedepth);
    printf("passes:\t\t%lld\n", args.passes);
    printf("sleep:\t\t%d\n", args.sleep);
    printf("measure:\t%d\n", args.measure);

    doit(args);
    return 0;
}
