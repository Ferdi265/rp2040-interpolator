from __future__ import annotations
from typing import List
from dataclasses import dataclass
from enum import Enum
from copy import copy, deepcopy
from random import randint

ubits = lambda n, b: n & ((1 << b) - 1)
sbits = lambda n, b: (-1 << (b - 1)) | ubits(n, b) if n & (1 << (b - 1)) else ubits(n, b)
u8 = lambda n: ubits(n, 8)
u16 = lambda n: ubits(n, 16)
u32 = lambda n: ubits(n, 32)
s32 = lambda n: sbits(n, 32)

class InterpGeneration(Enum):
    RP2040 = 0
    RP2350 = 1

@dataclass
class InterpCtrl:
    shift: int
    mask_lsb: int
    mask_msb: int
    is_signed: bool
    cross_input: bool
    cross_result: bool
    add_raw: bool
    force_msb: int
    blend: bool
    clamp: bool
    overf0: bool
    overf1: bool
    overf: bool
    _reserved0: int = 0

    @staticmethod
    def from_reg(value: int) -> 'InterpCtrl':
        return InterpCtrl(
            shift        =      (value >>  0) & 0b11111,
            mask_lsb     =      (value >>  5) & 0b11111,
            mask_msb     =      (value >> 10) & 0b11111,
            is_signed    = bool((value >> 15) & 1),
            cross_input  = bool((value >> 16) & 1),
            cross_result = bool((value >> 17) & 1),
            add_raw      = bool((value >> 18) & 1),
            force_msb    =      (value >> 19) & 0b11,
            blend        = bool((value >> 21) & 1),
            clamp        = bool((value >> 22) & 1),
            overf0       = bool((value >> 23) & 1),
            overf1       = bool((value >> 24) & 1),
            overf        = bool((value >> 25) & 1),
            _reserved0   =      (value >> 26) & 0b111111,
        )

    def to_reg(self) -> int:
        return (
            ((self.shift        & 0b11111)  <<  0) |
            ((self.mask_lsb     & 0b11111)  <<  5) |
            ((self.mask_msb     & 0b11111)  << 10) |
            ((self.is_signed    & 1)        << 15) |
            ((self.cross_input  & 1)        << 16) |
            ((self.cross_result & 1)        << 17) |
            ((self.add_raw      & 1)        << 18) |
            ((self.force_msb    & 0b11)     << 19) |
            ((self.blend        & 1)        << 21) |
            ((self.clamp        & 1)        << 22) |
            ((self.overf0       & 1)        << 23) |
            ((self.overf1       & 1)        << 24) |
            ((self.overf        & 1)        << 25) |
            ((self._reserved0   & 0b111111) << 26)
        )

@dataclass
class InterpState:
    accum: List[int] # len = 2
    base: List[int]  # len = 3
    ctrl: List[int]  # len = 2
    peek: List[int]   # len = 3
    peekraw: List[int] # len = 2

    @staticmethod
    def random() -> InterpState:
        rand32 = lambda: randint(0, 2**32-1)
        return InterpState(
            accum = [rand32(), rand32()],
            base = [rand32(), rand32(), rand32()],
            ctrl = [rand32(), rand32()],
            peek = [rand32(), rand32(), rand32()],
            peekraw = [rand32(), rand32()],
        )

    def __xor__(self, other: InterpState | any) -> InterpState:
        if not isinstance(other, InterpState):
            return NotImplemented

        diff = deepcopy(self)
        diff.accum[0] ^= other.accum[0]
        diff.accum[1] ^= other.accum[1]
        diff.base[0] ^= other.base[0]
        diff.base[1] ^= other.base[1]
        diff.base[2] ^= other.base[2]
        diff.ctrl[0] ^= other.ctrl[0]
        diff.ctrl[1] ^= other.ctrl[1]
        diff.peek[0] ^= other.peek[0]
        diff.peek[1] ^= other.peek[1]
        diff.peek[2] ^= other.peek[2]
        diff.peekraw[0] ^= other.peekraw[0]
        diff.peekraw[1] ^= other.peekraw[1]
        return diff

    def __bool__(self) -> bool:
        """
        Returns True if all values of the state are 0
        (e.g. the result of an xor of two identical states)
        """
        for value in (self.accum + self.base + self.ctrl + self.peek + self.peekraw):
            if value != 0:
                return False

        return True

    def __repr__(self) -> str:
        return (
            "InterpState("
            f"accum=[{self.accum[0]:#x}, {self.accum[1]:#x}], "
            f"base=[{self.base[0]:#x}, {self.base[1]:#x}, {self.base[2]:#x}], "
            f"ctrl=[{self.ctrl[0]:#x}, {self.ctrl[1]:#x}], "
            f"peek=[{self.peek[0]:#x}, {self.peek[1]:#x}, {self.peek[2]:#x}], "
            f"peekraw=[{self.peekraw[0]:#x}, {self.peekraw[1]:#x}]"
            ")"
        )

@dataclass
class Interp:
    n: int
    generation: InterpGeneration
    accum: List[int] # len = 2
    base: List[int]  # len = 3
    ctrl: List[int]  # len = 2

    _smresult: List[int] # len = 2
    _result: List[int]   # len = 3

    def __init__(self, n: int = 0, generation: InterpGeneration = InterpGeneration.RP2040):
        assert n in [0, 1], "invalid interpolator index"
        self.n = n
        self.generation = generation
        self.accum = [0, 0]
        self.base = [0, 0, 0]
        self.ctrl = [0, 0]
        self._smresult = [0, 0]
        self._result = [0, 0, 0]
        self.update()

    def pop(self, i: int) -> int:
        self.update()
        v = self._result[i]
        self._writeback()
        return v

    def peek(self, i: int) -> int:
        self.update()
        return self._result[i]

    def peekraw(self, i: int) -> int:
        self.update()
        return self._smresult[i]

    def add(self, i: int, value: int):
        self.accum[i] += value

    def base01(self, value: int):
        self._writebase01(value)

    def update(self):
        self._adjustbits()
        ctrl0 = InterpCtrl.from_reg(self.ctrl[0])
        ctrl1 = InterpCtrl.from_reg(self.ctrl[1])

        do_clamp = (ctrl0.clamp and self.n == 1)
        do_blend = (ctrl0.blend and self.n == 0)

        ctrl0.clamp = do_clamp
        ctrl0.blend = do_blend
        ctrl0._reserved0 = 0
        ctrl1.clamp = False
        ctrl1.blend = False
        ctrl1.overf0 = False
        ctrl1.overf1 = False
        ctrl1.overf = False
        ctrl1._reserved0 = 0

        input0 = self.accum[1 if ctrl0.cross_input else 0]
        input1 = self.accum[0 if ctrl1.cross_input else 1]

        mask0 = ((1 << (ctrl0.mask_msb + 1)) - 1) & ~((1 << ctrl0.mask_lsb) - 1)
        mask1 = ((1 << (ctrl1.mask_msb + 1)) - 1) & ~((1 << ctrl1.mask_lsb) - 1)

        match self.generation:
            case InterpGeneration.RP2040:
                uresult0 = (input0 >> ctrl0.shift) & mask0
                uresult1 = (input1 >> ctrl1.shift) & mask1
            case InterpGeneration.RP2350:
                uresult0 = ((input0 >> ctrl0.shift) | (input0 << (32 - ctrl0.shift))) & mask0;
                uresult1 = ((input1 >> ctrl1.shift) | (input1 << (32 - ctrl1.shift))) & mask1;

        match self.generation:
            case InterpGeneration.RP2040:
                overf0 = bool((input0 >> ctrl0.shift) & ~((1 << (ctrl0.mask_msb + 1)) - 1))
                overf1 = bool((input1 >> ctrl1.shift) & ~((1 << (ctrl1.mask_msb + 1)) - 1))
            case InterpGeneration.RP2350:
                overf0 = bool((input0 >> ctrl0.shift) | (input0 << (32 - ctrl0.shift))) & ~((1 << (ctrl0.mask_msb + 1)) - 1);
                overf1 = bool((input1 >> ctrl1.shift) | (input1 << (32 - ctrl1.shift))) & ~((1 << (ctrl1.mask_msb + 1)) - 1);
        overf = overf0 or overf1

        sextmask0 = (-1 << ctrl0.mask_msb) & ((1 << 32) - 1) if (uresult0 & (1 << ctrl0.mask_msb)) else 0
        sextmask1 = (-1 << ctrl1.mask_msb) & ((1 << 32) - 1) if (uresult1 & (1 << ctrl1.mask_msb)) else 0

        sresult0 = uresult0 | sextmask0
        sresult1 = uresult1 | sextmask1

        result0 = sresult0 if ctrl0.is_signed else uresult0
        result1 = sresult1 if ctrl1.is_signed else uresult1

        addresult0 = u32(self.base[0] + (input0 if ctrl0.add_raw else result0))
        addresult1 = u32(self.base[1] + (input1 if ctrl1.add_raw else result1))
        addresult2 = u32(self.base[2] + result0 + result1)

        uclamp0 = self.base[0] if result0 < self.base[0] else (self.base[1] if result0 > self.base[1] else result0)
        sclamp0 = self.base[0] if s32(result0) < s32(self.base[0]) else (self.base[1] if s32(result0) > s32(self.base[1]) else result0)
        clamp0 = u32(sclamp0) if ctrl0.is_signed else uclamp0

        alpha1 = u8(result1)
        ublend1 = self.base[0] + (alpha1 * (self.base[1] - self.base[0]) >> 8)
        sblend1 = s32(self.base[0]) + (alpha1 * (s32(self.base[1]) - s32(self.base[0])) >> 8)
        blend1 = u32(sblend1 if ctrl1.is_signed else ublend1)

        self._smresult[0] = result0
        self._smresult[1] = result1
        self._result[0] = alpha1 if do_blend else ((clamp0 if do_clamp else addresult0) | (ctrl0.force_msb << 28))
        self._result[1] = (blend1 if do_blend else addresult1) | (ctrl1.force_msb << 28)
        self._result[2] = u32(self.base[2] + result0) if do_blend else addresult2

        ctrl0.overf0 = overf0
        ctrl0.overf1 = overf1
        ctrl0.overf = overf
        self.ctrl[0] = ctrl0.to_reg()
        self.ctrl[1] = ctrl1.to_reg()

    def _writeback(self):
        ctrl0 = InterpCtrl.from_reg(self.ctrl[0])
        ctrl1 = InterpCtrl.from_reg(self.ctrl[1])

        self.accum[0] = self._result[1 if ctrl0.cross_result else 0]
        self.accum[1] = self._result[0 if ctrl1.cross_result else 1]

        self.update();

    def _writebase01(self, v: int):
        ctrl0 = InterpCtrl.from_reg(self.ctrl[0])
        ctrl1 = InterpCtrl.from_reg(self.ctrl[1])

        do_blend = (ctrl0.blend and self.n == 0)

        input0 = u16(v)
        input1 = u16(v >> 16)

        sextmask0 = u32(-1 << 15) if (input0 & (1 << 15)) else 0
        sextmask1 = u32(-1 << 15) if (input1 & (1 << 15)) else 0

        base0 = input0 | sextmask0 if (ctrl1.is_signed if do_blend else ctrl0.is_signed) else input0
        base1 = input1 | sextmask1 if ctrl1.is_signed else input1

        self.base[0] = base0;
        self.base[1] = base1;

        self.update()

    def _adjustbits(self):
        self.accum[0] = u32(self.accum[0])
        self.accum[1] = u32(self.accum[1])
        self.base[0]  = u32(self.base[0])
        self.base[1]  = u32(self.base[1])
        self.base[2]  = u32(self.base[2])
        self.ctrl[0]  = u32(self.ctrl[0])
        self.ctrl[1]  = u32(self.ctrl[1])

    def save(self) -> InterpState:
        self.update()
        return InterpState(
            accum = copy(self.accum),
            base = copy(self.base),
            ctrl = copy(self.ctrl),
            peek = [self._result[0], self._result[1], self._result[2]],
            peekraw = [self._smresult[0], self._smresult[1]]
        )

    def restore(self, state: InterpState):
        self.accum = copy(state.accum)
        self.base = copy(state.base)
        self.ctrl = copy(state.ctrl)
        self.update()
