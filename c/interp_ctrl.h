#ifndef YRLF_C_INTERP_CTRL_H_
#define YRLF_C_INTERP_CTRL_H_

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Register    : INTERP_SW_INDEX
// Description : Describes which interpolator index to emulate
#define INTERP_SW_INDEX_BLEND_CAPABLE 0
#define INTERP_SW_INDEX_CLAMP_CAPABLE 1
// -----------------------------------------------------------------------------
// Register    : INTERP_SW_GENERATION
// Description : Describes which generation of interpolator to emulate
#define INTERP_SW_GENERATION_RP2040 0
#define INTERP_SW_GENERATION_RP2350 1
#ifndef INTERP_SW_GENERATION_DEFAULT
#define INTERP_SW_GENERATION_DEFAULT INTERP_SW_GENERATION_RP2040
#endif
// =============================================================================

// =============================================================================
// Field       : INTERP_SW_CTRL_LANE0_OVERF
// Description : Set if either OVERF0 or OVERF1 is set.
#define INTERP_SW_CTRL_LANE0_OVERF_BITS   0x02000000u
#define INTERP_SW_CTRL_LANE0_OVERF_MSB    25u
#define INTERP_SW_CTRL_LANE0_OVERF_LSB    25u
// -----------------------------------------------------------------------------
// Field       : INTERP_SW_CTRL_LANE0_OVERF1
// Description : Indicates if any masked-off MSBs in ACCUM1 are set.
#define INTERP_SW_CTRL_LANE0_OVERF1_BITS   0x01000000u
#define INTERP_SW_CTRL_LANE0_OVERF1_MSB    24u
#define INTERP_SW_CTRL_LANE0_OVERF1_LSB    24u
// -----------------------------------------------------------------------------
// Field       : INTERP_SW_CTRL_LANE0_OVERF0
// Description : Indicates if any masked-off MSBs in ACCUM0 are set.
#define INTERP_SW_CTRL_LANE0_OVERF0_BITS   0x00800000u
#define INTERP_SW_CTRL_LANE0_OVERF0_MSB    23u
#define INTERP_SW_CTRL_LANE0_OVERF0_LSB    23u
// -----------------------------------------------------------------------------
// Field       : INTERP_SW_CTRL_LANE0_CLAMP
// Description : Only present on INTERP1 on each core. If CLAMP mode is enabled:
//               - LANE0 result is shifted and masked ACCUM0, clamped by a lower
//               bound of
//               BASE0 and an upper bound of BASE1.
//               - Signedness of these comparisons is determined by
//               LANE0_CTRL_SIGNED
#define INTERP_SW_CTRL_LANE0_CLAMP_BITS   0x00400000u
#define INTERP_SW_CTRL_LANE0_CLAMP_MSB    22u
#define INTERP_SW_CTRL_LANE0_CLAMP_LSB    22u
// -----------------------------------------------------------------------------
// Field       : INTERP_SW_CTRL_LANE0_BLEND
// Description : Only present on INTERP0 on each core. If BLEND mode is enabled:
//               - LANE1 result is a linear interpolation between BASE0 and
//               BASE1, controlled
//               by the 8 LSBs of lane 1 shift and mask value (a fractional
//               number between
//               0 and 255/256ths)
//               - LANE0 result does not have BASE0 added (yields only the 8
//               LSBs of lane 1 shift+mask value)
//               - FULL result does not have lane 1 shift+mask value added
//               (BASE2 + lane 0 shift+mask)
//               LANE1 SIGNED flag controls whether the interpolation is signed
//               or unsigned.
#define INTERP_SW_CTRL_LANE0_BLEND_BITS   0x00200000u
#define INTERP_SW_CTRL_LANE0_BLEND_MSB    21u
#define INTERP_SW_CTRL_LANE0_BLEND_LSB    21u
// -----------------------------------------------------------------------------
// Field       : INTERP_SW_CTRL_LANE0_FORCE_MSB
// Description : ORed into bits 29:28 of the lane result presented to the
//               processor on the bus.
//               No effect on the internal 32-bit datapath. Handy for using a
//               lane to generate sequence
//               of pointers into flash or SRAM.
#define INTERP_SW_CTRL_LANE0_FORCE_MSB_BITS   0x00180000u
#define INTERP_SW_CTRL_LANE0_FORCE_MSB_MSB    20u
#define INTERP_SW_CTRL_LANE0_FORCE_MSB_LSB    19u
// -----------------------------------------------------------------------------
// Field       : INTERP_SW_CTRL_LANE0_ADD_RAW
// Description : If 1, mask + shift is bypassed for LANE0 result. This does not
//               affect FULL result.
#define INTERP_SW_CTRL_LANE0_ADD_RAW_BITS   0x00040000u
#define INTERP_SW_CTRL_LANE0_ADD_RAW_MSB    18u
#define INTERP_SW_CTRL_LANE0_ADD_RAW_LSB    18u
// -----------------------------------------------------------------------------
// Field       : INTERP_SW_CTRL_LANE0_CROSS_RESULT
// Description : If 1, feed the opposite lane's result into this lane's
//               accumulator on POP.
#define INTERP_SW_CTRL_LANE0_CROSS_RESULT_BITS   0x00020000u
#define INTERP_SW_CTRL_LANE0_CROSS_RESULT_MSB    17u
#define INTERP_SW_CTRL_LANE0_CROSS_RESULT_LSB    17u
// -----------------------------------------------------------------------------
// Field       : INTERP_SW_CTRL_LANE0_CROSS_INPUT
// Description : If 1, feed the opposite lane's accumulator into this lane's
//               shift + mask hardware.
//               Takes effect even if ADD_RAW is set (the CROSS_INPUT mux is
//               before the shift+mask bypass)
#define INTERP_SW_CTRL_LANE0_CROSS_INPUT_BITS   0x00010000u
#define INTERP_SW_CTRL_LANE0_CROSS_INPUT_MSB    16u
#define INTERP_SW_CTRL_LANE0_CROSS_INPUT_LSB    16u
// -----------------------------------------------------------------------------
// Field       : INTERP_SW_CTRL_LANE0_SIGNED
// Description : If SIGNED is set, the shifted and masked accumulator value is
//               sign-extended to 32 bits
//               before adding to BASE0, and LANE0 PEEK/POP appear extended to
//               32 bits when read by processor.
#define INTERP_SW_CTRL_LANE0_SIGNED_BITS   0x00008000u
#define INTERP_SW_CTRL_LANE0_SIGNED_MSB    15u
#define INTERP_SW_CTRL_LANE0_SIGNED_LSB    15u
// -----------------------------------------------------------------------------
// Field       : INTERP_SW_CTRL_LANE0_MASK_MSB
// Description : The most-significant bit allowed to pass by the mask
//               (inclusive)
//               Setting MSB < LSB may cause chip to turn inside-out
#define INTERP_SW_CTRL_LANE0_MASK_MSB_BITS   0x00007c00u
#define INTERP_SW_CTRL_LANE0_MASK_MSB_MSB    14u
#define INTERP_SW_CTRL_LANE0_MASK_MSB_LSB    10u
// -----------------------------------------------------------------------------
// Field       : INTERP_SW_CTRL_LANE0_MASK_LSB
// Description : The least-significant bit allowed to pass by the mask
//               (inclusive)
#define INTERP_SW_CTRL_LANE0_MASK_LSB_BITS   0x000003e0u
#define INTERP_SW_CTRL_LANE0_MASK_LSB_MSB    9u
#define INTERP_SW_CTRL_LANE0_MASK_LSB_LSB    5u
// -----------------------------------------------------------------------------
// Field       : INTERP_SW_CTRL_LANE0_SHIFT
// Description : Logical right-shift applied to accumulator before masking
#define INTERP_SW_CTRL_LANE0_SHIFT_BITS   0x0000001fu
#define INTERP_SW_CTRL_LANE0_SHIFT_MSB    4u
#define INTERP_SW_CTRL_LANE0_SHIFT_LSB    0u
// =============================================================================

#ifdef __cplusplus
}
#endif

#endif
