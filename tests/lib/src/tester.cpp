#include "interp-test.h"

template struct InterpTester<InterpSW>;
#ifdef HAVE_RP2040_HARDWARE_INTERP
template struct InterpTester<InterpHW>;
#endif

template <template <size_t N> typename Interp>
void InterpTester<Interp>::write_state(interp_num_t n, const InterpState& state) {
    if (n == 0) {
        interp0 = state;
    } else if (n == 1) {
        interp1 = state;
    }
}

template <template <size_t N> typename Interp>
void InterpTester<Interp>::dump_state(interp_num_t n, InterpState& state) {
    if (n == 0) {
        state = interp0;
    } else if (n == 1) {
        state = interp1;
    }
}

template <typename Interp>
static void do_write_reg(Interp& interp, InterpReg reg, uint32_t value) {
    switch (reg) {
        case InterpReg::ACCUM0: interp.accum[0] = value; break;
        case InterpReg::ACCUM1: interp.accum[1] = value; break;
        case InterpReg::BASE0: interp.base[0] = value; break;
        case InterpReg::BASE1: interp.base[1] = value; break;
        case InterpReg::BASE2: interp.base[2] = value; break;
        case InterpReg::CTRL0: interp.ctrl[0] = value; break;
        case InterpReg::CTRL1: interp.ctrl[1] = value; break;
        case InterpReg::ADD0: interp.add(0, value); break;
        case InterpReg::ADD1: interp.add(1, value); break;
        case InterpReg::BASE01: interp.base01(value); break;
        default:;
    }
}

template <template <size_t N> typename Interp>
void InterpTester<Interp>::write_reg(interp_num_t n, InterpReg reg, uint32_t value) {
    if (n == 0) {
        do_write_reg(interp0, reg, value);
    } else if (n == 1) {
        do_write_reg(interp1, reg, value);
    }
}

template <typename Interp>
static void do_read_reg(Interp& interp, InterpReg reg, uint32_t& value) {
    switch (reg) {
        case InterpReg::ACCUM0: value = interp.accum[0]; break;
        case InterpReg::ACCUM1: value = interp.accum[1]; break;
        case InterpReg::BASE0: value = interp.base[0]; break;
        case InterpReg::BASE1: value = interp.base[1]; break;
        case InterpReg::BASE2: value = interp.base[2]; break;
        case InterpReg::CTRL0: value = interp.ctrl[0]; break;
        case InterpReg::CTRL1: value = interp.ctrl[1]; break;
        case InterpReg::POP0: value = interp.pop(0); break;
        case InterpReg::POP1: value = interp.pop(1); break;
        case InterpReg::POP2: value = interp.pop(2); break;
        case InterpReg::PEEK0: value = interp.peek(0); break;
        case InterpReg::PEEK1: value = interp.peek(1); break;
        case InterpReg::PEEK2: value = interp.peek(2); break;
        case InterpReg::PEEKRAW0: value = interp.peekraw(0); break;
        case InterpReg::PEEKRAW1: value = interp.peekraw(1); break;
        default:;
    }
}

template <template <size_t N> typename Interp>
void InterpTester<Interp>::read_reg(interp_num_t n, InterpReg reg, uint32_t& value) {
    if (n == 0) {
        do_read_reg(interp0, reg, value);
    } else if (n == 1) {
        do_read_reg(interp1, reg, value);
    }
}
