#ifndef YRLF_C_INTERP_H_
#define YRLF_C_INTERP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include "interp_ctrl.h"

/** \brief Interpolator software definition
 *  \defgroup interp_config interp_config
 *  \ingroup hardware_interp
 *
 * Two software interpolator instances compatible with the hardware equivalent:
 * interp0_sw and interp1_sw
 */

typedef struct {
    uint32_t accum[2];
    uint32_t base[3];
    uint32_t ctrl[2];
    uint32_t peek[3];
    uint32_t peekraw[2];
    uint16_t index;
    uint16_t generation;
} interp_sw_t;

extern interp_sw_t interp0_sw;
extern interp_sw_t interp1_sw;

/*! \brief Update the simulated interpolator
 *  \ingroup interp_config
 *
 * Updates the cached results of the simulated interpolator.
 *
 * \param interp Interpolator instance, interp0 or interp1.
 */
void interp_sw_update(interp_sw_t *interp);

/*! \brief Trigger the writeback operation on a simulated interpolator
 *  \ingroup interp_config
 *
 * Performs the writeback operation that occurs after reading the POP registers.
 *
 * \param interp Interpolator instance, interp0 or interp1.
 */
void interp_sw_writeback(interp_sw_t *interp);

/** \brief Interpolator configuration
 *  \defgroup interp_config interp_config
 *  \ingroup hardware_interp
 *
 * Each interpolator needs to be configured, these functions provide handy helpers to set up configuration
 * structures.
 *
 */

typedef struct {
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
} interp_sw_config_t;

/*! \brief Construct an interp_sw_config value from a ctrl register
 *  \ingroup interp_config
 *
 *
 * \param c Pointer to an interpolator config
 * \param a ctrl register value
 */
static inline void interp_sw_config_from_reg(interp_sw_config_t *c, uint32_t ctrl) {
    _Static_assert(sizeof (interp_sw_config_t) == sizeof (uint32_t), "interp_sw_config_t has invalid layout");
    memcpy(c, &ctrl, sizeof (interp_sw_config_t));
}

/*! \brief Convert an interp_sw_config value to a ctrl register
 *  \ingroup interp_config
 *
 *
 * \param c Pointer to an interpolator config
 * \return a ctrl register value
 */
static inline uint32_t interp_sw_config_to_reg(interp_sw_config_t *c) {
    _Static_assert(sizeof (interp_sw_config_t) == sizeof (uint32_t), "interp_sw_config_t has invalid layout");
    uint32_t ctrl = 0;
    memcpy(&ctrl, c, sizeof (interp_sw_config_t));
    return ctrl;
}

/*! \brief Set the interpolator shift value
 *  \ingroup interp_config
 *
 * Sets the number of bits the accumulator is shifted before masking, on each iteration.
 *
 * \param c Pointer to an interpolator config
 * \param shift Number of bits
 */
static inline void interp_sw_config_set_shift(interp_sw_config_t *c, uint32_t shift) {
    c->shift = shift;
}

/*! \brief Set the interpolator mask range
 *  \ingroup interp_config
 *
 * Sets the range of bits (least to most) that are allowed to pass through the interpolator
 *
 * \param c Pointer to interpolation config
 * \param mask_lsb The least significant bit allowed to pass
 * \param mask_msb The most significant bit allowed to pass
 */
static inline void interp_sw_config_set_mask(interp_sw_config_t *c, uint32_t mask_lsb, uint32_t mask_msb) {
    c->mask_lsb = mask_lsb;
    c->mask_msb = mask_msb;
}

/*! \brief Enable cross input
 *  \ingroup interp_config
 *
 *  Allows feeding of the accumulator content from the other lane back in to this lanes shift+mask hardware.
 *  This will take effect even if the interp_config_set_add_raw option is set as the cross input mux is before the
 *  shift+mask bypass
 *
 * \param c Pointer to interpolation config
 * \param cross_input If true, enable the cross input.
 */
static inline void interp_sw_config_set_cross_input(interp_sw_config_t *c, bool cross_input) {
    c->cross_input = cross_input;
}

/*! \brief Enable cross results
 *  \ingroup interp_config
 *
 *  Allows feeding of the other lane’s result into this lane’s accumulator on a POP operation.
 *
 * \param c Pointer to interpolation config
 * \param cross_result If true, enables the cross result
 */
static inline void interp_sw_config_set_cross_result(interp_sw_config_t *c, bool cross_result) {
    c->cross_result = cross_result;
}

/*! \brief Set sign extension
 *  \ingroup interp_config
 *
 * Enables signed mode, where the shifted and masked accumulator value is sign-extended to 32 bits
 * before adding to BASE1, and LANE1 PEEK/POP results appear extended to 32 bits when read by processor.
 *
 * \param c Pointer to interpolation config
 * \param  _signed If true, enables sign extension
 */
static inline void interp_sw_config_set_signed(interp_sw_config_t *c, bool _signed) {
    c->is_signed = _signed;
}

/*! \brief Set raw add option
 *  \ingroup interp_config
 *
 * When enabled, mask + shift is bypassed for LANE0 result. This does not affect the FULL result.
 *
 * \param c Pointer to interpolation config
 * \param add_raw If true, enable raw add option.
 */
static inline void interp_sw_config_set_add_raw(interp_sw_config_t *c, bool add_raw) {
    c->add_raw = add_raw;
}

/*! \brief Set blend mode
 *  \ingroup interp_config
 *
 * If enabled, LANE1 result is a linear interpolation between BASE0 and BASE1, controlled
 * by the 8 LSBs of lane 1 shift and mask value (a fractional number between 0 and 255/256ths)
 *
 * LANE0 result does not have BASE0 added (yields only the 8 LSBs of lane 1 shift+mask value)
 *
 * FULL result does not have lane 1 shift+mask value added (BASE2 + lane 0 shift+mask)
 *
 * LANE1 SIGNED flag controls whether the interpolation is signed or unsig
 *
 * \param c Pointer to interpolation config
 * \param blend Set true to enable blend mode.
*/
static inline void interp_sw_config_set_blend(interp_sw_config_t *c, bool blend) {
    c->blend = blend;
}

/*! \brief Set interpolator clamp mode (Interpolator 1 only)
 *  \ingroup interp_config
 *
 * Only present on INTERP1 on each core. If CLAMP mode is enabled:
 * - LANE0 result is a shifted and masked ACCUM0, clamped by a lower bound of BASE0 and an upper bound of BASE1.
 * - Signedness of these comparisons is determined by LANE0_CTRL_SIGNED
 *
 * \param c Pointer to interpolation config
 * \param clamp Set true to enable clamp mode
 */
static inline void interp_sw_config_set_clamp(interp_sw_config_t *c, bool clamp) {
    c->clamp = clamp;
}

/*! \brief Set interpolator Force bits
 *  \ingroup interp_config
 *
 * ORed into bits 29:28 of the lane result presented to the processor on the bus.
 *
 * No effect on the internal 32-bit datapath. Handy for using a lane to generate sequence
 * of pointers into flash or SRAM
 *
 * \param c Pointer to interpolation config
 * \param bits Sets the force bits to that specified. Range 0-3 (two bits)
 */
static inline void interp_sw_config_set_force_bits(interp_sw_config_t *c, uint32_t bits) {
    c->force_msb = bits;
}

/*! \brief Get a default configuration
 *  \ingroup interp_config
 *
 * \return A default interpolation configuration
 */
static inline interp_sw_config_t interp_sw_default_config(void) {
    interp_sw_config_t c;
    interp_sw_config_from_reg(&c, 0);
    // Just pass through everything
    interp_sw_config_set_mask(&c, 0, 31);
    return c;
}

/*! \brief Send configuration to a lane
 *  \ingroup interp_config
 *
 * If an invalid configuration is specified (ie a lane specific item is set on wrong lane),
 * depending on setup this function can panic.
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \param lane The lane to set
 * \param config Pointer to interpolation config
 */

static inline void interp_sw_set_config(interp_sw_t *interp, uint32_t lane, interp_sw_config_t *config) {
    interp->ctrl[lane] = interp_sw_config_to_reg(config);
}

/*! \brief Directly set the force bits on a specified lane
 *  \ingroup hardware_interp
 *
 * These bits are ORed into bits 29:28 of the lane result presented to the processor on the bus.
 * There is no effect on the internal 32-bit datapath.
 *
 * Useful for using a lane to generate sequence of pointers into flash or SRAM, saving a subsequent
 * OR or add operation.
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \param lane The lane to set
 * \param bits The bits to set (bits 0 and 1, value range 0-3)
 */
static inline void interp_sw_set_force_bits(interp_sw_t *interp, uint32_t lane, uint32_t bits) {
    // note cannot use hw_set_bits on SIO
    interp->ctrl[lane] = interp->ctrl[lane] | (bits << INTERP_SW_CTRL_LANE0_FORCE_MSB_LSB);
}

typedef struct {
    uint32_t accum[2];
    uint32_t base[3];
    uint32_t ctrl[2];
    uint32_t peek[3];
    uint32_t peekraw[2];
} interp_sw_save_t;

/*! \brief Save the specified interpolator state
 *  \ingroup hardware_interp
 *
 * Can be used to save state if you need an interpolator for another purpose, state
 * can then be recovered afterwards and continue from that point
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \param saver Pointer to the save structure to fill in
 */
void interp_sw_save(interp_sw_t *interp, interp_sw_save_t *saver);

/*! \brief Restore an interpolator state
 *  \ingroup hardware_interp
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \param saver Pointer to save structure to reapply to the specified interpolator
 */
void interp_sw_restore(interp_sw_t *interp, interp_sw_save_t *saver);

/*! \brief Sets the interpolator base register by lane
 *  \ingroup hardware_interp
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \param lane The lane number, 0 or 1 or 2
 * \param val The value to apply to the register
 */
static inline void interp_sw_set_base(interp_sw_t *interp, uint32_t lane, uint32_t val) {
    interp->base[lane] = val;
}

/*! \brief Gets the content of interpolator base register by lane
 *  \ingroup hardware_interp
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \param lane The lane number, 0 or 1 or 2
 * \return  The current content of the lane base register
 */
static inline uint32_t interp_sw_get_base(interp_sw_t *interp, uint32_t lane) {
    return interp->base[lane];
}

/*! \brief Sets the interpolator base registers simultaneously
 *  \ingroup hardware_interp
 *
 *  The lower 16 bits go to BASE0, upper bits to BASE1 simultaneously.
 *  Each half is sign-extended to 32 bits if that lane’s SIGNED flag is set.
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \param val The value to apply to the register
 */
void interp_sw_set_base_both(interp_sw_t *interp, uint32_t val);


/*! \brief Sets the interpolator accumulator register by lane
 *  \ingroup hardware_interp
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \param lane The lane number, 0 or 1
 * \param val The value to apply to the register
 */
static inline void interp_sw_set_accumulator(interp_sw_t *interp, uint32_t lane, uint32_t val) {
    interp->accum[lane] = val;
}

/*! \brief Gets the content of the interpolator accumulator register by lane
 *  \ingroup hardware_interp
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \param lane The lane number, 0 or 1
 * \return The current content of the register
 */
static inline uint32_t interp_sw_get_accumulator(interp_sw_t *interp, uint32_t lane) {
    return interp->accum[lane];
}

/*! \brief Read lane result, and write lane results to both accumulators to update the interpolator
 *  \ingroup hardware_interp
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \param lane The lane number, 0 or 1
 * \return The content of the lane result register
 */
static inline uint32_t interp_sw_pop_lane_result(interp_sw_t *interp, uint32_t lane) {
    interp_sw_update(interp);
    uint32_t result = interp->peek[lane];
    interp_sw_writeback(interp);
    return result;
}

/*! \brief Read lane result
 *  \ingroup hardware_interp
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \param lane The lane number, 0 or 1
 * \return The content of the lane result register
 */
static inline uint32_t interp_sw_peek_lane_result(interp_sw_t *interp, uint32_t lane) {
    interp_sw_update(interp);
    return interp->peek[lane];
}

/*! \brief Read lane result, and write lane results to both accumulators to update the interpolator
 *  \ingroup hardware_interp
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \return The content of the FULL register
 */
static inline uint32_t interp_sw_pop_full_result(interp_sw_t *interp) {
    interp_sw_update(interp);
    uint32_t result = interp->peek[2];
    interp_sw_writeback(interp);
    return result;
}

/*! \brief Read lane result
 *  \ingroup hardware_interp
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \return The content of the FULL register
 */
static inline uint32_t interp_sw_peek_full_result(interp_sw_t *interp) {
    interp_sw_update(interp);
    return interp->peek[2];
}

/*! \brief Add to accumulator
 *  \ingroup hardware_interp
 *
 * Atomically add the specified value to the accumulator on the specified lane
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \param lane The lane number, 0 or 1
 * \param val Value to add
 */
static inline void interp_sw_add_accumulator(interp_sw_t *interp, uint32_t lane, uint32_t val) {
    interp->accum[lane] += val;
}

/*! \brief Get raw lane value
 *  \ingroup hardware_interp
 *
 * Returns the raw shift and mask value from the specified lane, BASE0 is NOT added
 *
 * \param interp Interpolator instance, interp0 or interp1.
 * \param lane The lane number, 0 or 1
 * \return The raw shift/mask value
 */
static inline uint32_t interp_sw_get_raw(interp_sw_t *interp, uint32_t lane) {
    interp_sw_update(interp);
    return interp->peekraw[lane];
}

#ifdef __cplusplus
}
#endif

#endif
