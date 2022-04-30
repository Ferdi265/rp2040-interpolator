#ifndef YRLF_INTERP_TEST_H_
#define YRLF_INTERP_TEST_H_

#include <string_view>
#include "interp.h"

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
    Interp<0> interp0;
    Interp<1> interp1;

    void write_state(interp_num_t, const InterpState&) override;
    void dump_state(interp_num_t, InterpState&) override;
    void write_reg(interp_num_t, InterpReg, uint32_t) override;
    void read_reg(interp_num_t, InterpReg, uint32_t&) override;
};

using InterpSWTester = InterpTester<InterpSW>;
#ifdef HAVE_RP2040_HARDWARE_INTERP
using InterpHWTester = InterpTester<InterpHW>;
#endif

#endif
