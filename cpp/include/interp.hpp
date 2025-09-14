#ifndef YRLF_INTERP_HPP_
#define YRLF_INTERP_HPP_

#include <cstdint>
#include <bit>

enum struct InterpGeneration {
    RP2040,
    RP2350,
#ifdef RP2040_INTERP_GENERATION_RP2350
    DEFAULT = RP2350,
#else
    DEFAULT = RP2040,
#endif
};

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

    static InterpCtrl from(uint32_t v) { return std::bit_cast<InterpCtrl>(v); }
    uint32_t to() const { return std::bit_cast<uint32_t>(*this); }
};

struct InterpState {
    uint32_t accum[2];
    uint32_t base[3];
    uint32_t ctrl[2];
    uint32_t peek[3];
    uint32_t peekraw[2];

    InterpState() = default;
    InterpState(const InterpState&) = default;

    friend bool operator<=>(InterpState, InterpState) = default;
};

#ifndef YRLF_INTERP_SW_HPP_
#include "interp-sw.hpp"
#endif

#if RP2040_INTERP_WITH_C
#ifndef YRLF_INTERP_SW_C_HPP_
#include "interp-sw-c.hpp"
#endif
#endif

#if RP2040_INTERP_WITH_HARDWARE
#ifndef YRLF_INTERP_HW_HPP_
#include "interp-hw.hpp"
#endif

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

#endif
