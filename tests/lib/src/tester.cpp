#include "interp-test.h"

template struct InterpTester<InterpSW>;
#if RP2040_INTERP_WITH_HARDWARE
template struct InterpTester<InterpHW>;
#endif

template <template <size_t N> typename Interp>
void InterpTester<Interp>::write_state(interp_num_t n, const InterpState& state) {
    if (n == 0) {
        intrp0 = state;
    } else if (n == 1) {
        intrp1 = state;
    }
}

template <template <size_t N> typename Interp>
void InterpTester<Interp>::dump_state(interp_num_t n, InterpState& state) {
    if (n == 0) {
        state = intrp0;
    } else if (n == 1) {
        state = intrp1;
    }
}

template <typename Interp>
static void do_write_reg(Interp& intrp, InterpReg reg, uint32_t value) {
    switch (reg) {
        case InterpReg::ACCUM0: intrp.accum[0] = value; break;
        case InterpReg::ACCUM1: intrp.accum[1] = value; break;
        case InterpReg::BASE0: intrp.base[0] = value; break;
        case InterpReg::BASE1: intrp.base[1] = value; break;
        case InterpReg::BASE2: intrp.base[2] = value; break;
        case InterpReg::CTRL0: intrp.ctrl[0] = value; break;
        case InterpReg::CTRL1: intrp.ctrl[1] = value; break;
        case InterpReg::ADD0: intrp.add(0, value); break;
        case InterpReg::ADD1: intrp.add(1, value); break;
        case InterpReg::BASE01: intrp.base01(value); break;
        default:;
    }
}

template <template <size_t N> typename Interp>
void InterpTester<Interp>::write_reg(interp_num_t n, InterpReg reg, uint32_t value) {
    if (n == 0) {
        do_write_reg(intrp0, reg, value);
    } else if (n == 1) {
        do_write_reg(intrp1, reg, value);
    }
}

template <typename Interp>
static void do_read_reg(Interp& intrp, InterpReg reg, uint32_t& value) {
    switch (reg) {
        case InterpReg::ACCUM0: value = intrp.accum[0]; break;
        case InterpReg::ACCUM1: value = intrp.accum[1]; break;
        case InterpReg::BASE0: value = intrp.base[0]; break;
        case InterpReg::BASE1: value = intrp.base[1]; break;
        case InterpReg::BASE2: value = intrp.base[2]; break;
        case InterpReg::CTRL0: value = intrp.ctrl[0]; break;
        case InterpReg::CTRL1: value = intrp.ctrl[1]; break;
        case InterpReg::POP0: value = intrp.pop(0); break;
        case InterpReg::POP1: value = intrp.pop(1); break;
        case InterpReg::POP2: value = intrp.pop(2); break;
        case InterpReg::PEEK0: value = intrp.peek(0); break;
        case InterpReg::PEEK1: value = intrp.peek(1); break;
        case InterpReg::PEEK2: value = intrp.peek(2); break;
        case InterpReg::PEEKRAW0: value = intrp.peekraw(0); break;
        case InterpReg::PEEKRAW1: value = intrp.peekraw(1); break;
        default:;
    }
}

template <template <size_t N> typename Interp>
void InterpTester<Interp>::read_reg(interp_num_t n, InterpReg reg, uint32_t& value) {
    if (n == 0) {
        do_read_reg(intrp0, reg, value);
    } else if (n == 1) {
        do_read_reg(intrp1, reg, value);
    }
}
