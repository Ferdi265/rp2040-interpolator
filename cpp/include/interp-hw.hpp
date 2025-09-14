#ifndef YRLF_INTERP_HW_HPP_
#define YRLF_INTERP_HW_HPP_

#include <cstddef>
#include <cstdint>
#include <hardware/interp.h>

#ifndef YRLF_INTERP_HPP_
#include <interp.hpp>
#endif

template <typename T, size_t addr>
struct reg_proxy {
    using ptr_type = volatile T*;
    using ref_type = volatile T&;
    constexpr ref_type get() { return *(ptr_type)addr; }
    constexpr operator ref_type() { return get(); }
};

template <size_t N = 0>
struct InterpHW {
private:
    static_assert(N == 0 || N == 1, "invalid interpolator index");
    constexpr static size_t INTERP_BASE = SIO_BASE + (sizeof (interp_hw_t) * N);

public:
    reg_proxy<io_rw_32[2], INTERP_BASE + SIO_INTERP0_ACCUM0_OFFSET> accum;
    reg_proxy<io_rw_32[3], INTERP_BASE + SIO_INTERP0_BASE0_OFFSET> base;
    reg_proxy<io_rw_32[2], INTERP_BASE + SIO_INTERP0_CTRL_LANE0_OFFSET> ctrl;

    uint32_t pop(size_t i) { return hw_pop[i]; }
    uint32_t peek(size_t i) { return hw_peek[i]; }
    uint32_t peekraw(size_t i) { return hw_add[i]; }
    void add(size_t i, uint32_t v) { hw_add[i] = v; }
    void base01(uint32_t v) { hw_base01.get() = v; }
    uint32_t read_base01() { return hw_base01.get(); }
    void update() {}

    InterpHW& operator=(const InterpState& state) { restore(state); return *this; }
    operator InterpState() { InterpState state; save(state); return state; }
    void save(InterpState& state);
    void restore(const InterpState& state);

private:
    reg_proxy<io_ro_32[3], INTERP_BASE + SIO_INTERP0_POP_LANE0_OFFSET> hw_pop;
    reg_proxy<io_ro_32[3], INTERP_BASE + SIO_INTERP0_PEEK_LANE0_OFFSET> hw_peek;
    reg_proxy<io_rw_32[3], INTERP_BASE + SIO_INTERP0_ACCUM0_ADD_OFFSET> hw_add;
    reg_proxy<io_wo_32, INTERP_BASE + SIO_INTERP0_BASE_1AND0_OFFSET> hw_base01;
};

using InterpHW0 = InterpHW<0>;
using InterpHW1 = InterpHW<1>;

// --- implementation ---

template <size_t N>
void InterpHW<N>::save(InterpState& state) {
    state.ctrl[0] = ctrl[0];
    state.ctrl[1] = ctrl[1];
    state.accum[0] = accum[0];
    state.accum[1] = accum[1];
    state.base[0] = base[0];
    state.base[1] = base[1];
    state.base[2] = base[2];
    state.peek[0] = peek(0);
    state.peek[1] = peek(1);
    state.peek[2] = peek(2);
    state.peekraw[0] = peekraw(0);
    state.peekraw[1] = peekraw(1);
}

template <size_t N>
void InterpHW<N>::restore(const InterpState& state) {
    ctrl[0] = state.ctrl[0];
    ctrl[1] = state.ctrl[1];
    accum[0] = state.accum[0];
    accum[1] = state.accum[1];
    base[0] = state.base[0];
    base[1] = state.base[1];
    base[2] = state.base[2];
}

#endif
