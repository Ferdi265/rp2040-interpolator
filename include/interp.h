#ifndef YRLF_INTERP_H_
#define YRLF_INTERP_H_

#include <stddef.h>
#include <stdint.h>
#include "interp-util.h"

#if RP2040_INTERP_WITH_HARDWARE
#include "hardware/interp.h"
#endif

struct InterpCtrl {
    uint32_t shift : 5;
    uint32_t mask_lsb : 5;
    uint32_t mask_msb : 5;
    bool is_signed : 1;
    bool cross_input : 1;
    bool cross_result : 1;
    bool add_raw : 1;
    uint32_t force_msb : 2;
    bool blend : 1;
    bool clamp : 1;
    bool overf0 : 1;
    bool overf1 : 1;
    bool overf : 1;
    uint32_t _reserved0 : 6;

    static InterpCtrl from(uint32_t v) { return bit_cast<InterpCtrl, uint32_t>(v); }
    uint32_t to() { return bit_cast<uint32_t, InterpCtrl>(*this); }
};

template <size_t N>
struct InterpSW;

#if RP2040_INTERP_WITH_HARDWARE
template <size_t N>
struct InterpHW;
#endif

struct InterpState {
    uint32_t accum[2];
    uint32_t base[3];
    uint32_t ctrl[2];
    uint32_t peek[3];
    uint32_t peekraw[2];

    InterpState() = default;
    InterpState(const InterpState&) = default;
    template <size_t M>
    InterpState(const InterpSW<M>& interp) { save(interp); }
    InterpState& operator=(const InterpState&) = default;
    template <size_t M>
    InterpState& operator=(const InterpSW<M>& interp) { save(interp); return *this; }

    template <size_t M>
    void save(const InterpSW<M>&);
    template <size_t M>
    void restore(InterpSW<M>&) const;

#if RP2040_INTERP_WITH_HARDWARE
    template <size_t M>
    InterpState(InterpHW<M>& interp) { save(interp); }
    template <size_t M>
    InterpState& operator=(InterpHW<M>& interp) { save(interp); return *this; }

    template <size_t M>
    void save(InterpHW<M>&);
    template <size_t M>
    void restore(InterpHW<M>&) const;
#endif
};

template <size_t N = 0>
struct InterpSW {
private:
    friend struct InterpState;
    static_assert(N == 0 || N == 1, "invalid interpolator index");

public:
    uint32_t accum[2];
    uint32_t base[3];
    uint32_t ctrl[2];

    uint32_t pop(size_t i) { update(); uint32_t v = result[i]; writeback(); return v; }
    uint32_t peek(size_t i) { update(); return result[i]; }
    uint32_t peekraw(size_t i) { update(); return smresult[i]; }
    void add(size_t i, uint32_t v) { accum[i] += v; }
    void base01(uint32_t v) { writebase01(v); }
    void update();

    InterpSW& operator=(const InterpState& state) { state.restore(*this); return *this; }

private:
    void writeback();
    void writebase01(uint32_t v);

    uint32_t smresult[2];
    uint32_t result[3];
};

using InterpSW0 = InterpSW<0>;
using InterpSW1 = InterpSW<1>;

#if RP2040_INTERP_WITH_HARDWARE
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
    void update() {}

    InterpHW& operator=(const InterpState& interp) { interp.restore(*this); return *this; }

private:
    reg_proxy<io_ro_32[3], INTERP_BASE + SIO_INTERP0_POP_LANE0_OFFSET> hw_pop;
    reg_proxy<io_ro_32[3], INTERP_BASE + SIO_INTERP0_PEEK_LANE0_OFFSET> hw_peek;
    reg_proxy<io_rw_32[3], INTERP_BASE + SIO_INTERP0_ACCUM0_ADD_OFFSET> hw_add;
    reg_proxy<io_wo_32, INTERP_BASE + SIO_INTERP0_BASE_1AND0_OFFSET> hw_base01;
};

using InterpHW0 = InterpHW<0>;
using InterpHW1 = InterpHW<1>;
#endif

#if RP2040_INTERP_WITH_HARDWARE
template <size_t N = 0>
using Interp = InterpHW<N>;
using Interp0 = InterpHW0;
using Interp1 = InterpHW1;
#else
template <size_t N = 0>
using Interp = InterpSW<N>;
using Interp0 = InterpSW0;
using Interp1 = InterpSW1;
#endif

#ifndef YRLF_INTERP_STATE_HPP_
#include "interp-state.hpp"
#endif

#ifndef YRLF_INTERP_SW_HPP_
#include "interp-sw.hpp"
#endif

#endif
