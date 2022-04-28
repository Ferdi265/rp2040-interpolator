#ifndef YRLF_INTERP_SW_HPP_
#define YRLF_INTERP_SW_HPP_

#ifndef YRLF_INTERP_H_
#include "interp.h"
#endif

template <size_t N>
void InterpSW<N>::update() {
    InterpCtrl ctrl0 = InterpCtrl::from(ctrl[0]);
    InterpCtrl ctrl1 = InterpCtrl::from(ctrl[1]);

    bool do_clamp = (ctrl0.clamp && N == 1);
    bool do_blend = (ctrl0.blend && N == 0);

    ctrl0.clamp = do_clamp;
    ctrl0.blend = do_blend;
    ctrl0._reserved0 = 0;
    ctrl1.clamp = 0;
    ctrl1.blend = 0;
    ctrl1.overf0 = 0;
    ctrl1.overf1 = 0;
    ctrl1.overf = 0;
    ctrl1._reserved0 = 0;

    uint32_t input0 = accum[ctrl0.cross_input ? 1 : 0];
    uint32_t input1 = accum[ctrl1.cross_input ? 0 : 1];

    uint32_t mask0 = ((1L << (ctrl0.mask_msb + 1)) - 1) & ~((1L << ctrl0.mask_lsb) - 1);
    uint32_t mask1 = ((1L << (ctrl1.mask_msb + 1)) - 1) & ~((1L << ctrl1.mask_lsb) - 1);

    uint32_t uresult0 = (input0 >> ctrl0.shift) & mask0;
    uint32_t uresult1 = (input1 >> ctrl1.shift) & mask1;

    bool overf0 = (input0 >> ctrl0.shift) & ~((1L << (ctrl0.mask_msb + 1)) - 1);
    bool overf1 = (input1 >> ctrl1.shift) & ~((1L << (ctrl1.mask_msb + 1)) - 1);
    bool overf = overf0 || overf1;

    uint32_t sextmask0 = (uresult0 & (1 << ctrl0.mask_msb)) ? (-1U << ctrl0.mask_msb) : 0;
    uint32_t sextmask1 = (uresult1 & (1 << ctrl1.mask_msb)) ? (-1U << ctrl1.mask_msb) : 0;

    uint32_t sresult0 = uresult0 | sextmask0;
    uint32_t sresult1 = uresult1 | sextmask1;

    uint32_t result0 = ctrl0.is_signed ? sresult0 : uresult0;
    uint32_t result1 = ctrl1.is_signed ? sresult1 : uresult1;

    uint32_t addresult0 = base[0] + (ctrl0.add_raw ? input0 : result0);
    uint32_t addresult1 = base[1] + (ctrl1.add_raw ? input1 : result1);
    uint32_t addresult2 = base[2] + result0 + result1;

    auto s32 = sext<int32_t>;
    uint32_t uclamp0 = result0 < base[0] ? base[0] : (result0 > base[1] ? base[1] : result0);
    uint32_t sclamp0 = s32(result0) < s32(base[0]) ? base[0] : (s32(result0) > s32(base[1]) ? base[1] : result0);
    uint32_t clamp0 = ctrl0.is_signed ? sclamp0 : uclamp0;

    auto u64 = zext<uint64_t>;
    auto s64 = sext<int64_t>;
    uint8_t alpha1 = result1;
    uint32_t ublend1 = base[0] + (alpha1 * (u64(base[1]) - u64(base[0])) >> 8);
    uint32_t sblend1 = base[0] + (alpha1 * (s64(base[1]) - s64(base[0])) >> 8);
    uint32_t blend1 = ctrl1.is_signed ? sblend1 : ublend1;

    smresult[0] = result0;
    smresult[1] = result1;
    result[0] = do_blend ? alpha1 : (do_clamp ? clamp0 : addresult0) | (ctrl0.force_msb << 28);
    result[1] = (do_blend ? blend1 : addresult1) | (ctrl1.force_msb << 28);
    result[2] = do_blend ? base[2] + result0 : addresult2;

    ctrl0.overf0 = overf0;
    ctrl0.overf1 = overf1;
    ctrl0.overf = overf;
    ctrl[0] = ctrl0.to();
    ctrl[1] = ctrl1.to();
}

template <size_t N>
void InterpSW<N>::writeback() {
    InterpCtrl ctrl0 = InterpCtrl::from(ctrl[0]);
    InterpCtrl ctrl1 = InterpCtrl::from(ctrl[1]);

    accum[0] = result[ctrl0.cross_result ? 1 : 0];
    accum[1] = result[ctrl1.cross_result ? 0 : 1];

    update();
}

template <size_t N>
void InterpSW<N>::writebase01(uint32_t v) {
    InterpCtrl ctrl0 = InterpCtrl::from(ctrl[0]);
    InterpCtrl ctrl1 = InterpCtrl::from(ctrl[1]);

    bool do_blend = (ctrl0.blend && N == 0);

    uint16_t input0 = v;
    uint16_t input1 = v >> 16;

    uint32_t sextmask0 = (input0 & (1 << 15)) ? (-1U << 15) : 0;
    uint32_t sextmask1 = (input1 & (1 << 15)) ? (-1U << 15) : 0;

    uint32_t base0 = (do_blend ? ctrl1.is_signed : ctrl0.is_signed) ? input0 | sextmask0 : input0;
    uint32_t base1 = ctrl1.is_signed ? input1 | sextmask1 : input1;

    base[0] = base0;
    base[1] = base1;

    update();
}

#endif
