#include <interp-test.hpp>

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
        intrp0.update();
        state = intrp0;
    } else if (n == 1) {
        intrp1.update();
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
    intrp.update();
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
        case InterpReg::ADD0: value = intrp.peekraw(0); break;
        case InterpReg::ADD1: value = intrp.peekraw(1); break;
        case InterpReg::BASE01: value = intrp.read_base01(); break;
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

#if RP2040_INTERP_WITH_HARDWARE
void InterpDualTester::write_state(interp_num_t n, const InterpState& state) {
    sw.write_state(n, state);
    hw.write_state(n, state);
}

void InterpDualTester::dump_state(interp_num_t n, InterpState& state) {
    InterpState sw_state, hw_state;
    sw.dump_state(n, sw_state);
    hw.dump_state(n, hw_state);

    if (sw_state != hw_state) {
        throw InterpDualTestStateFailure(n, sw_state, hw_state);
    }

    state = sw_state;
}

void InterpDualTester::write_reg(interp_num_t n, InterpReg r, uint32_t v) {
    sw.write_reg(n, r, v);
    hw.write_reg(n, r, v);

    InterpState state;
    dump_state(n, state);
}

void InterpDualTester::read_reg(interp_num_t n, InterpReg r, uint32_t& v) {
    InterpState before_state;
    sw.dump_state(n, before_state);

    uint32_t sw_v, hw_v;
    sw.read_reg(n, r, sw_v);
    hw.read_reg(n, r, hw_v);

    InterpState state;
    dump_state(n, state);

    if (sw_v != hw_v) {
        throw InterpDualTestValueFailure(n, before_state, sw_v, hw_v);
    }

    v = sw_v;
}
#endif
