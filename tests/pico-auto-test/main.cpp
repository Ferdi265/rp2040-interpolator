#include <print>
#include <format>
#include "pico/stdio_usb.h"
#include "pico/rand.h"
#include "pico/time.h"
#include "interp-test.h"

template <typename Char>
struct std::formatter<InterpState, Char> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const InterpState& s, FormatContext& ctx) const {
        return std::format_to(ctx.out(), 
            "State(accum=[{:#x}, {:#x}], base=[{:#x}, {:#x}, {:#x}], ctrl=[{:#x}, {:#x}], "
            "peek=[{:#x}, {:#x}, {:#x}], peekraw=[{:#x}, {:#x}])",
            s.accum[0], s.accum[1],
            s.base[0], s.base[1], s.base[2],
            s.ctrl[0], s.ctrl[1],
            s.peek[0], s.peek[1], s.peek[2],
            s.peekraw[0], s.peekraw[1]
        );
    }
};

template <typename Char>
struct std::formatter<InterpReg, Char> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const InterpReg& r, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "{}",
            r == InterpReg::ACCUM0 ? "ACCUM0" :
            r == InterpReg::ACCUM1 ? "ACCUM1" :
            r == InterpReg::BASE0 ? "BASE0" :
            r == InterpReg::BASE1 ? "BASE1" :
            r == InterpReg::BASE2 ? "BASE2" :
            r == InterpReg::CTRL0 ? "CTRL0" :
            r == InterpReg::CTRL1 ? "CTRL1" :
            r == InterpReg::POP0 ? "POP0" :
            r == InterpReg::POP1 ? "POP1" :
            r == InterpReg::POP2 ? "POP2" :
            r == InterpReg::PEEK0 ? "PEEK0" :
            r == InterpReg::PEEK1 ? "PEEK1" :
            r == InterpReg::PEEK2 ? "PEEK2" :
            r == InterpReg::PEEKRAW0 ? "PEEKRAW0" :
            r == InterpReg::PEEKRAW1 ? "PEEKRAW1" :
            r == InterpReg::ADD0 ? "ADD0" :
            r == InterpReg::ADD1 ? "ADD1" :
            r == InterpReg::BASE01 ? "BASE01" :
            "???"
        );
    }
};

static interp_num_t random_interp_num() {
    return (interp_num_t)(get_rand_32() % 2);
}

static InterpState random_state() {
    InterpState s;
    s.accum[0] = get_rand_32();
    s.accum[1] = get_rand_32();
    s.base[0] = get_rand_32();
    s.base[1] = get_rand_32();
    s.base[2] = get_rand_32();
    s.ctrl[0] = get_rand_32();
    s.ctrl[1] = get_rand_32();
    s.peek[0] = get_rand_32();
    s.peek[1] = get_rand_32();
    s.peek[2] = get_rand_32();
    s.peekraw[0] = get_rand_32();
    s.peekraw[1] = get_rand_32();
    return s;
}

static InterpReg random_reg() {
    return InterpReg(get_rand_32() % (int(InterpReg::BASE01) + 1));
}

static void write_random_state(InterpTesterBase& tester) {
    interp_num_t n = random_interp_num();
    InterpState s = random_state();
    std::println(">> write_random_state: n={}, state={}", int(n), s);
    tester.write_state(n, s);
}

static void write_random_reg(InterpTesterBase& tester) {
    interp_num_t n = random_interp_num();
    InterpReg r = random_reg();
    uint32_t v = get_rand_32();
    std::println(">> write_random_reg: n={}, reg={}, value={:#x}", int(n), r, v);
    tester.write_reg(n, r, v);
}

static void read_random_reg(InterpTesterBase& tester) {
    interp_num_t n = random_interp_num();
    InterpReg r = random_reg();
    std::println(">> read_random_reg: n={}, reg={}", int(n), r);
    uint32_t v;
    tester.read_reg(n, r, v);
}

static void random_action(InterpTesterBase& tester) {
    switch (get_rand_32() % 3) {
        case 0: return write_random_state(tester);
        case 1: return write_random_reg(tester);
        case 2: return read_random_reg(tester);
    }
}

int main() {
    stdio_usb_init();

    sleep_ms(1000);
    std::println("starting...");
    sleep_ms(1000);

    try {
        InterpDualTester tester;
        InterpState initial_state = random_state();
        std::println(">> write_initial_state: state={}", initial_state);
        tester.write_state(0, initial_state);
        tester.write_state(1, initial_state);
        while (true) {
            random_action(tester);
        }
    } catch (const InterpDualTestStateFailure& e) {
        std::println("{}: n={}, sw={} hw={}", e.what(), int(e.n), e.sw_state, e.hw_state);
    } catch (const InterpDualTestValueFailure& e) {
        std::println("{}: n={}, state={}, sw={:#x} hw={:#x}", e.what(), int(e.n), e.state, e.sw_value, e.hw_value);
    }

    while (true) {}
}
