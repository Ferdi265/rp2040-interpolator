#ifndef YRLF_INTERP_STATE_HPP_
#define YRLF_INTERP_STATE_HPP_

#ifndef YRLF_INTERP_H_
#include "interp.h"
#endif

template <size_t M>
void InterpState::save(const InterpSW<M>& sw) {
    ctrl[0] = sw.ctrl[0];
    ctrl[1] = sw.ctrl[1];
    accum[0] = sw.accum[0];
    accum[1] = sw.accum[1];
    base[0] = sw.base[0];
    base[1] = sw.base[1];
    base[2] = sw.base[2];
    peek[0] = sw.result[0];
    peek[1] = sw.result[1];
    peek[2] = sw.result[2];
    peekraw[0] = sw.smresult[0];
    peekraw[1] = sw.smresult[1];
}

template <size_t M>
void InterpState::restore(InterpSW<M>& sw) const {
    sw.ctrl[0] = ctrl[0];
    sw.ctrl[1] = ctrl[1];
    sw.accum[0] = accum[0];
    sw.accum[1] = accum[1];
    sw.base[0] = base[0];
    sw.base[1] = base[1];
    sw.base[2] = base[2];
    sw.result[0] = peek[0];
    sw.result[1] = peek[1];
    sw.result[2] = peek[2];
    sw.smresult[0] = peekraw[0];
    sw.smresult[1] = peekraw[1];
}

#ifdef HAVE_RP2040_HARDWARE_INTERP
template <size_t M>
void InterpState::save(InterpHW<M>& hw) {
    ctrl[0] = hw.ctrl[0];
    ctrl[1] = hw.ctrl[1];
    accum[0] = hw.accum[0];
    accum[1] = hw.accum[1];
    base[0] = hw.base[0];
    base[1] = hw.base[1];
    base[2] = hw.base[2];
    peek[0] = hw.peek(0);
    peek[1] = hw.peek(1);
    peek[2] = hw.peek(2);
    peekraw[0] = hw.peekraw(0);
    peekraw[1] = hw.peekraw(1);
}

template <size_t M>
void InterpState::restore(InterpHW<M>& hw) const {
    hw.ctrl[0] = ctrl[0];
    hw.ctrl[1] = ctrl[1];
    hw.accum[0] = accum[0];
    hw.accum[1] = accum[1];
    hw.base[0] = base[0];
    hw.base[1] = base[1];
    hw.base[2] = base[2];
}
#endif

#endif
