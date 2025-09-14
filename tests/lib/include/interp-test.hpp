#ifndef YRLF_INTERP_TEST_H_
#define YRLF_INTERP_TEST_H_

#include <string_view>
#include <stdexcept>
#include <interp.hpp>

enum struct InterpReg {
    ACCUM0,
    ACCUM1,
    BASE0,
    BASE1,
    BASE2,
    CTRL0,
    CTRL1,
    POP0,
    POP1,
    POP2,
    PEEK0,
    PEEK1,
    PEEK2,
    PEEKRAW0,
    PEEKRAW1,
    ADD0,
    ADD1,
    BASE01
};

using interp_num_t = bool;

struct InterpTesterBase {
    std::string_view parse_command(std::string_view cmd);

    virtual void write_state(interp_num_t, const InterpState&) = 0;
    virtual void dump_state(interp_num_t, InterpState&) = 0;
    virtual void write_reg(interp_num_t, InterpReg, uint32_t) = 0;
    virtual void read_reg(interp_num_t, InterpReg, uint32_t&) = 0;
};

template <template <size_t N> typename Interp = Interp>
struct InterpTester : InterpTesterBase {
    Interp<0> intrp0;
    Interp<1> intrp1;

    void write_state(interp_num_t, const InterpState&) override;
    void dump_state(interp_num_t, InterpState&) override;
    void write_reg(interp_num_t, InterpReg, uint32_t) override;
    void read_reg(interp_num_t, InterpReg, uint32_t&) override;
};

#if RP2040_INTERP_WITH_HARDWARE
template <template <size_t N> typename InterpSW = InterpSW>
struct InterpDualTester : InterpTesterBase {
    InterpTester<InterpSW> sw;
    InterpTester<InterpHW> hw;

    void write_state(interp_num_t, const InterpState&) override;
    void dump_state(interp_num_t, InterpState&) override;
    void write_reg(interp_num_t, InterpReg, uint32_t) override;
    void read_reg(interp_num_t, InterpReg, uint32_t&) override;
};

struct InterpDualTestFailure : std::runtime_error {
    InterpDualTestFailure(const char* msg) : std::runtime_error(msg) {}
};

struct InterpDualTestStateFailure : InterpDualTestFailure {
    interp_num_t n;
    InterpState sw_state;
    InterpState hw_state;

    InterpDualTestStateFailure(interp_num_t n, const InterpState& sw, const InterpState& hw) : InterpDualTestFailure("InterpDualTest state failure"), n(n), sw_state(sw), hw_state(hw) {}
};
struct InterpDualTestValueFailure : InterpDualTestFailure {
    interp_num_t n;
    InterpState state;
    uint32_t sw_value;
    uint32_t hw_value;

    InterpDualTestValueFailure(interp_num_t n, const InterpState& state, uint32_t sw, uint32_t hw) : InterpDualTestFailure("InterpDualTest value failure"), n(n), state(state), sw_value(sw), hw_value(hw) {}
};
#endif
using InterpSWTester = InterpTester<InterpSW>;
#if RP2040_INTERP_WITH_HARDWARE
using InterpHWTester = InterpTester<InterpHW>;
#endif

#endif
