#include <interp.h>
#include <interp_ctrl.h>

interp_sw_t interp0_sw = { .index = INTERP_SW_INDEX_BLEND_CAPABLE, .generation = INTERP_SW_GENERATION_DEFAULT };
interp_sw_t interp1_sw = { .index = INTERP_SW_INDEX_CLAMP_CAPABLE, .generation = INTERP_SW_GENERATION_DEFAULT };

void interp_sw_update(interp_sw_t *interp) {
    interp_sw_config_t ctrl0, ctrl1;
    interp_sw_config_from_reg(&ctrl0, interp->ctrl[0]);
    interp_sw_config_from_reg(&ctrl1, interp->ctrl[1]);

    bool do_clamp = (ctrl0.clamp && interp->index == INTERP_SW_INDEX_CLAMP_CAPABLE);
    bool do_blend = (ctrl1.blend && interp->index == INTERP_SW_INDEX_BLEND_CAPABLE);

    ctrl0.clamp = do_clamp;
    ctrl0.blend = do_blend;
    ctrl0._reserved0 = 0;
    ctrl1.clamp = 0;
    ctrl1.blend = 0;
    ctrl1.overf0 = 0;
    ctrl1.overf1 = 0;
    ctrl1.overf = 0;
    ctrl1._reserved0 = 0;

    uint32_t input0 = interp->accum[ctrl0.cross_input ? 1 : 0];
    uint32_t input1 = interp->accum[ctrl1.cross_input ? 0 : 1];

    uint32_t mask0 = ((1LL << (ctrl0.mask_msb + 1)) - 1) & ~((1LL << ctrl0.mask_lsb) - 1);
    uint32_t mask1 = ((1LL << (ctrl1.mask_msb + 1)) - 1) & ~((1LL << ctrl1.mask_lsb) - 1);

    uint32_t shift0;
    uint32_t shift1;
    switch (interp->generation) {
        default:
        case INTERP_SW_GENERATION_RP2040:
            shift0 = input0 >> ctrl0.shift;
            shift1 = input1 >> ctrl1.shift;
            break;
        case INTERP_SW_GENERATION_RP2350:
            shift0 = (input0 >> ctrl0.shift) | ((uint64_t)input0 << (32 - ctrl0.shift));
            shift1 = (input1 >> ctrl1.shift) | ((uint64_t)input1 << (32 - ctrl1.shift));
            break;
    }

    uint32_t uresult0 = shift0 & mask0;
    uint32_t uresult1 = shift1 & mask1;

    bool overf0 = shift0 & ~((1LL << (ctrl0.mask_msb + 1)) - 1);
    bool overf1 = shift1 & ~((1LL << (ctrl1.mask_msb + 1)) - 1);
    bool overf = overf0 || overf1;

    uint32_t sextmask0 = (shift0 & (1 << ctrl0.mask_msb)) ? (-1U << (ctrl0.mask_msb + 1)) : 0;
    uint32_t sextmask1 = (shift1 & (1 << ctrl1.mask_msb)) ? (-1U << (ctrl1.mask_msb + 1)) : 0;

    uint32_t sresult0 = uresult0 | sextmask0;
    uint32_t sresult1 = uresult1 | sextmask1;

    uint32_t result0 = ctrl0.is_signed ? sresult0 : uresult0;
    uint32_t result1 = ctrl1.is_signed ? sresult1 : uresult1;

    uint32_t addresult0 = interp->base[0] + (ctrl0.add_raw ? input0 : result0);
    uint32_t addresult1 = interp->base[1] + (ctrl1.add_raw ? input1 : result1);
    uint32_t addresult2 = interp->base[2] + result0 + (do_blend ? 0 : result1);

    uint32_t uclamp0 = result0 < interp->base[0] ? interp->base[0] : (result0 > interp->base[1] ? interp->base[1] : result0);
    uint32_t sclamp0 = (int32_t)result0 < (int32_t)interp->base[0] ? interp->base[0] : ((int32_t)result0 > (int32_t)interp->base[1] ? interp->base[1] : result0);
    uint32_t clamp0 = ctrl0.is_signed ? sclamp0 : uclamp0;

    uint8_t alpha1 = result1;
    uint32_t ublend1 = interp->base[0] + (alpha1 * ((uint64_t)interp->base[1] - (uint64_t)interp->base[0]) >> 8);
    uint32_t sblend1 = interp->base[0] + (alpha1 * ((int64_t)(int32_t)interp->base[1] - (int64_t)(int32_t)interp->base[0]) >> 8);
    uint32_t blend1 = ctrl1.is_signed ? sblend1 : ublend1;

    interp->peekraw[0] = result0;
    interp->peekraw[1] = result1;
    interp->peek[0] = do_blend ? alpha1 : (do_clamp ? clamp0 : addresult0) | (ctrl0.force_msb << 28);
    interp->peek[1] = (do_blend ? blend1 : addresult1) | (ctrl1.force_msb << 28);
    interp->peek[2] = addresult2;

    ctrl0.overf0 = overf0;
    ctrl0.overf1 = overf1;
    ctrl0.overf = overf;
    interp->ctrl[0] = interp_sw_config_to_reg(&ctrl0);
    interp->ctrl[1] = interp_sw_config_to_reg(&ctrl1);
}

void interp_sw_writeback(interp_sw_t *interp) {
    interp_sw_config_t ctrl0, ctrl1;
    interp_sw_config_from_reg(&ctrl0, interp->ctrl[0]);
    interp_sw_config_from_reg(&ctrl1, interp->ctrl[1]);

    interp->accum[0] = interp->peek[ctrl0.cross_result ? 1 : 0];
    interp->accum[1] = interp->peek[ctrl1.cross_result ? 0 : 1];

    interp_sw_update(interp);
}

void interp_sw_set_base_both(interp_sw_t *interp, uint32_t val) {
    interp_sw_config_t ctrl0, ctrl1;
    interp_sw_config_from_reg(&ctrl0, interp->ctrl[0]);
    interp_sw_config_from_reg(&ctrl1, interp->ctrl[1]);

    bool do_blend = (ctrl1.blend && interp->index == INTERP_SW_INDEX_BLEND_CAPABLE);

    uint16_t input0 = val;
    uint16_t input1 = val >> 16;

    uint32_t sextmask0 = (input0 & (1 << 15)) ? (-1U << 15) : 0;
    uint32_t sextmask1 = (input1 & (1 << 15)) ? (-1U << 15) : 0;

    uint32_t base0 = (do_blend ? ctrl1.is_signed : ctrl0.is_signed) ? input0 | sextmask0 : input0;
    uint32_t base1 = ctrl1.is_signed ? input1 | sextmask1 : input1;

    interp->base[0] = base0;
    interp->base[1] = base1;

    interp_sw_update(interp);
}

void interp_sw_save(interp_sw_t *interp, interp_sw_save_t *saver) {
    interp_sw_update(interp);

    saver->accum[0] = interp->accum[0];
    saver->accum[1] = interp->accum[1];
    saver->base[0] = interp->base[0];
    saver->base[1] = interp->base[1];
    saver->base[2] = interp->base[2];
    saver->ctrl[0] = interp->ctrl[0];
    saver->ctrl[1] = interp->ctrl[1];
    saver->peek[0] = interp->peek[0];
    saver->peek[1] = interp->peek[1];
    saver->peek[2] = interp->peek[2];
    saver->peekraw[0] = interp->peekraw[0];
    saver->peekraw[1] = interp->peekraw[1];
}

void interp_sw_restore(interp_sw_t *interp, interp_sw_save_t *saver) {
    interp->accum[0] = saver->accum[0];
    interp->accum[1] = saver->accum[1];
    interp->base[0] = saver->base[0];
    interp->base[1] = saver->base[1];
    interp->base[2] = saver->base[2];
    interp->ctrl[0] = saver->ctrl[0];
    interp->ctrl[1] = saver->ctrl[1];
    interp->peek[0] = saver->peek[0];
    interp->peek[1] = saver->peek[1];
    interp->peek[2] = saver->peek[2];
    interp->peekraw[0] = saver->peekraw[0];
    interp->peekraw[1] = saver->peekraw[1];

    interp_sw_update(interp);
}
