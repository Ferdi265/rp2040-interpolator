#ifndef YRLF_INTERP_SW_C_HPP_
#define YRLF_INTERP_SW_C_HPP_

#include <cstddef>
#include <cstdint>
#include "interp.h"
#include "interp_ctrl.h"

#ifndef YRLF_INTERP_HPP_
#include <interp.hpp>
#endif

template <size_t N = 0, InterpGeneration G = InterpGeneration::DEFAULT>
struct InterpSWC {
private:
    static_assert(N == 0 || N == 1, "invalid interpolator index");

public:
    InterpSWC();
    InterpSWC(const InterpSWC&) = default;
    InterpSWC(InterpSWC&&) = default;

    uint32_t accum[2];
    uint32_t base[3];
    uint32_t ctrl[2];

    uint32_t pop(size_t i);
    uint32_t peek(size_t i);
    uint32_t peekraw(size_t i);
    void add(size_t i, uint32_t v);
    void base01(uint32_t v);
    uint32_t read_base01() { return 0; }
    void update();

    InterpSWC& operator=(const InterpState& state) { restore(state); return *this; }
    operator InterpState() { InterpState state; save(state); return state; }
    void save(InterpState& state);
    void restore(const InterpState& state);

private:
    interp_sw_t interp;
};

using InterpSWC0 = InterpSWC<0>;
using InterpSWC1 = InterpSWC<1>;

// --- implementation ---

template <size_t N, InterpGeneration G>
InterpSWC<N, G>::InterpSWC() {
    interp.index = N;
    interp.generation = G == InterpGeneration::RP2040 ? INTERP_SW_GENERATION_RP2040 : INTERP_SW_GENERATION_RP2350;
}

template <size_t N, InterpGeneration G>
uint32_t InterpSWC<N, G>::pop(size_t i) {
    if (i != 2) {
        return interp_sw_pop_lane_result(&interp, i);
    } else {
        return interp_sw_pop_full_result(&interp);
    }
}

template <size_t N, InterpGeneration G>
uint32_t InterpSWC<N, G>::peek(size_t i) {
    if (i != 2) {
        return interp_sw_peek_lane_result(&interp, i);
    } else {
        return interp_sw_peek_full_result(&interp);
    }
}

template <size_t N, InterpGeneration G>
uint32_t InterpSWC<N, G>::peekraw(size_t i) {
    return interp_sw_get_raw(&interp, i);
}

template <size_t N, InterpGeneration G>
void InterpSWC<N, G>::add(size_t i, uint32_t v) {
    interp_sw_add_accumulator(&interp, i, v);
}

template <size_t N, InterpGeneration G>
void InterpSWC<N, G>::base01(uint32_t v) {
    interp_sw_set_base_both(&interp, v);
}

template <size_t N, InterpGeneration G>
void InterpSWC<N, G>::update() {
    interp_sw_update(&interp);
}

template <size_t N, InterpGeneration G>
void InterpSWC<N, G>::save(InterpState& state) {
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

template <size_t N, InterpGeneration G>
void InterpSWC<N, G>::restore(const InterpState& state) {
    ctrl[0] = state.ctrl[0];
    ctrl[1] = state.ctrl[1];
    accum[0] = state.accum[0];
    accum[1] = state.accum[1];
    base[0] = state.base[0];
    base[1] = state.base[1];
    base[2] = state.base[2];
}

#endif
