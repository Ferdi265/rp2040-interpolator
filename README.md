# `rp2040-interpolator`

Software Emulation Library for the RP2040 Interpolator peripheral.

Note! The RP2350's Interpolator is almost perfectly compatible with the RP2040,
but its behaviour is not identical. The only difference is a right rotate
instead of a right shift (which also makes the OVERF flags break due to checking the value of the top bits regardless of where they came from). See
the RP2350 datasheet's Interpolator section and the RP2350-E1 Erratum. This
library was tested on both RP2040 and RP2350 and has bitwise correct results
for all tested cases. A list of standalone test vectors generated from RP2040
and RP2350 hardware will be provided soon.

## The Interpolator

The Interpolator is a fun little peripheral integrated into the SIO block in
the RP2040 and RP2350. It's only accessible from the CPU, but can do some
useful calculations when you want to calculate indices into tables, textures,
interpolate or clamp values, or maybe do some SNES MODE7-style graphics.

Each CPU has access to 2 Interpolators with 2 lanes of shift/mask/add hardware
each. INTERP0 also supports "blend mode" (linear interpolation), while INTERP1
supports "clamp mode".

Since the Interpolator is a quite unique piece of hardware and the datasheet is
not very explicit on some of the nuances of its behaviour, it might be useful
to have a software simulator for it so you can check whether your config for it
does what you thought it would without needing to try it on real hardware.

### Registers

```
INTERP_BASE:
    - uint32_t accum[2] (rw)
    - uint32_t base[3] (rw)
    - uint32_t pop[3] (ro)
    - uint32_t peek[3] (ro)
    - uint32_t ctrl[2] (rw)
    - uint32_t add[2] (wo) / uint32_t raw[2] (ro)
    - uint32_t base01 (wo)
```

### Diagram

```
                                                                                                                +------------+                                            
                                                                                                                | CLAMP UNIT | CTRL0.clamp                                
                            CTRL0.cross_input                                       CTRL0.signed  CTRL0.add_raw +------------+       | CTRL0.force_msb                    
                    +--------+       |                                                        |        |          ^ ^ ^    |       _ |     |    CTRL0.blend               
 CTRL0.cross_result |  BASE0 |-------------------------------------------------------------------------------+----+ | |    |      | \|     |      _ |                     
              _ |   +--------+       |                                                        |     | \|     |    | | |    \----->|1 \     v     | \|                     
             | \|                  _ |    /-------------------------------------------------------->|1 \     v    | | |           |  |--> OR --->|0 \        +---------+  
  RESULT0 -->|0 \   +--------+    | \|    |                                                 _ |     |  |--> ADD ----------------->|0 /           |  |------->| RESULT0 |  
             |  |-->| ACCUM0 |--->|0 \    |  +-------------+   +------+     +----------+   | \|  /->|0 /          | | |           |_/    /------>|1 /        +---------+  
  RESULT1 -->|1 /   +--------+    |  |----+->| Right Shift*|-->| Mask |--+--| Sign Ext |-->|1 \  |  |_/           | | |                  |       |_/                      
             |_/      ACCUM1 ---->|1 /       +-------------+   +------+  |  +----------+   |  |--+       +------+ | | |  /---------------/                                
                                  |_/                                    \---------------->|0 /  \------>| RAW0 |-----+--------------------------------\                  
                    +--------+                                                             |_/           +------+ | |    |                             v     +---------+  
                    |  BASE2 |----------------------------------------------------------------------------------------------------------------------- ADD -->| RESULT2 |  
                    +--------+     _                                                       | \           +------+ | |    |                       | \   ^     +---------+  
              _                   | \                                    /---------------->|0 \  /------>| RAW1 |-----+------------------------->|0 \  |                  
             | \      ACCUM0 ---->|1 \       +-------------+   +------+  |  +----------+   |  |--+   _   +------+ | | |  |                       |  |--/                  
  RESULT1 -->|0 \   +--------+    |  |----+->| Right Shift*|-->| Mask |--+--| Sign Ext |-->|1 /  |  | \           | | |  |         _        0 -->|1 /                     
             |  |-->| ACCUM1 |--->|0 /    |  +-------------+   +------+     +----------+   |_/|  \->|0 \          | | |  |        | \            |_/|                     
  RESULT0 -->|1 /   +--------+    |_/|    |                                                   |     |  |--> ADD ----------------->|0 \              |        +---------+  
             |_/|                    |    \-------------------------------------------------------->|1 /     ^    | | |  |        |  |--> OR --------------->| RESULT1 |  
                |   +--------+       |                                                        |     |_/|     |    | | |  | /----->|1 /     ^        |        +---------+  
 CTRL1.cross_result |  BASE1 |-------------------------------------------------------------------------------+------+ |  | |      |_/|     |        |                     
                    +--------+       |                                                        |        |          v v v  | |         |     |    CTRL0.blend               
                            CTRL1.cross_input                                       CTRL1.signed  CTRL1.add_raw +------------+       | CTRL1.force_msb                    
                                                                                                                | BLEND UNIT | CTRL0.blend                                
                                                                                                                +------------+                                            
```

Note: each Interpolator only has either a CLAMP or a BLEND unit. This diagram
shows both to avoid needing to draw 2 diagrams.

Note \*: The RP2350 has a Right Rotate unit here instead of a right shift unit.
This causes the OVERF flags to be of little use on the RP2350 if a nonzero
shift amount is used.

## C++ Library

The `CMakeLists.txt` in the `cpp` directory defines a header-only library
`rp2040-interp-cpp` with main header `include/interp.hpp`.

### `<interp.hpp>`

- `enum InterpGeneration`: identifies the Interpolator variant
  - `RP2040`
  - `RP2350`
  - `DEFAULT` (`RP2040`, or `RP2350` when `RP2040_INTERP_GENERATION_RP2350` is set`)

- `struct InterpCtrl`: Interpolator lane settings bitfield
  - `uint32_t shift : 5`
  - `uint32_t mask_lsb : 5`
  - `uint32_t mask_msb : 5`
  - `bool is_signed : 1`
  - `bool cross_input : 1`
  - `bool cross_result : 1`
  - `bool add_raw : 1`
  - `uint32_t force_msb : 2`
  - `bool blend : 1`
  - `bool clamp : 1`
  - `bool overf0 : 1`
  - `bool overf1 : 1`
  - `bool overf : 1`
  - `static InterpCtrl from(uint32_t)`: convert from packed form
  - `uint32_t to() const`: convert to packed form

- `struct InterpState`: Snapshot of Interpolator state
  - `uint32_t accum[2]`
  - `uint32_t base[3]`
  - `uint32_t ctrl[2]`
  - `uint32_t peek[3]`
  - `uint32_t peekraw[2]`
  - `InterpState() = default`
  - `InterpState(const InterpState&) = default`
  - `InterpState(const InterpSW<N, G>&)`: save state from a simulated Interpolator instance
  - `InterpState(const InterpHW<N>&)`: save state from a hardware Interpolator instance
  - `InterpState& operator=(const InterpState&) = default`
  - `InterpState& operator=(const InterpSW<N, G>&)`: save state from a simulated Interpolator instance
  - `InterpState& operator=(const InterpHW<N>&)`: save state from a hardware Interpolator instance
  - `void save(const InterpSW<N, G>&)`:  save state from a simulated Interpolator instance
  - `void save(const InterpHW<N>&)`:  save state from a hardware Interpolator instance
  - `void restore(const InterpSW<N, G>&) const`:  restore state to a simulated Interpolator instance
  - `void restore(const InterpHW<N>&) const`:  restore state to a hardware Interpolator instance

- `struct InterpSW<size_t N, InterpGeneration G = InterpGeneration::DEFAULT>`: Software Simulation of an Interpolator
  - N must be 0 or 1 and describes which interpolator instance is used
  - G must be a variant of InterpGeneration and describes which generation of Interpolator is simulated
  - `uint32_t accum[2]`
  - `uint32_t base[3]`
  - `uint32_t ctrl[2]`
  - `uint32_t pop(size_t i)`: simulate read from `POP_LANE0` (i=0), `POP_LANE1` (i=1), or `POP_FULL` (i=2) registers
  - `uint32_t peek(size_t i)`: simulate read from `PEEK_LANE0` (i=0), `PEEK_LANE1` (i=1), or `PEEK_FULL` (i=2) registers
  - `uint32_t peekraw(size_t i)`: simulate read from `ACCUM0_ADD` (i=0) or `ACCUM1_ADD` (i=1) registers
  - `void add(size_t i, uint32_t v)`: simulate write to `ACCUM0_ADD` (i=0) or `ACCUM1_ADD` (i=1) registers
  - `void base01(uint32_t v)`: simulate write to `BASE_1AND0` registers
  - `void update()`: update result (automatically called internally)

- `struct InterpHW<size_t N>`: Hardware Wrapper with same API as `InterpSW<N>`
  - only available when `RP2040_INTERP_WITH_HARDWARE` is set

- `InterpSW0`: alias for `InterpSW<0>`
- `InterpSW1`: alias for `InterpSW<1>`
- `InterpHW0`: alias for `InterpHW<0>`
- `InterpHW1`: alias for `InterpHW<1>`
- `Interp<N>`: alias for `InterpSW<N>`, or `InterpHW<N>` when `RP2040_INTERP_WITH_HARDWARE` is set
- `Interp0`: alias for `Interp<0>`
- `Interp1`: alias for `Interp<1>`

## C Library

The `CMakeLists.txt` in the `c` directory defines a static library
`rp2040-interp-c` with main header `include/interp.h`.

### `<interp.h>`

TODO: document

## Python Library

The `python/` folder contains the python package `rp2040_interp`.

- `class InterpGeneration(Enum)`: identifies the Interpolator variant
  - `RP2040`
  - `RP2350`

- `class InterpCtrl`: Interpolator lane settings dataclass
  - `shift: int`
  - `mask_lsb: int`
  - `mask_msb: int`
  - `is_signed: bool`
  - `cross_input: bool`
  - `cross_result: bool`
  - `add_raw: bool`
  - `force_msb: int`
  - `blend: bool`
  - `clamp: bool`
  - `overf0: bool`
  - `overf1: bool`
  - `overf: bool`
  - `def from_reg(value: int) -> InterpCtrl`: convert from packed form
  - `def to_reg(self) -> int`: convert to packed form

- `class InterpState`: Interpolator state dataclass
  - `accum: List[int] # len = 2`
  - `base: List[int]  # len = 3`
  - `ctrl: List[int]  # len = 2`
  - `peek: List[int]  # len = 3`
  - `peekraw: List[int]  # len = 2`
  - `def random() -> InterpState` (static): generate a random interpolator state
  - xor operator: compare two interpolator states
  - bool operator: check if the result of comparing two interpolator states is
    all zero (i.e. identical states)

- `class Interp`: Software Simulation of an Interpolator
  - `def __init__(self, n: int = 0, generation: InterpGeneration = InterpGeneration.RP2040)`: constructor
    - n must be 0 or 1 and describes which interpolator instance is used
    - generation must be a variant of InterpGeneration and describes which generation of Interpolator is simulated
  - `accum: List[int] # len = 2`
  - `base: List[int]  # len = 3`
  - `ctrl: List[int]  # len = 2`
  - `def set_accum(i: int, v: int)`: set an accumulator register and update
  - `def set_base(i: int, v: int)`: set a base register and update
  - `def set_ctrl(i: int, v: int)`: set a ctrl register and update
  - `def pop(i: int) -> int`: simulate read from `POP_LANE0` (i=0), `POP_LANE1` (i=1), or `POP_FULL` (i=2) registers
  - `def peek(i: int) -> int`: simulate read from `PEEK_LANE0` (i=0), `PEEK_LANE1` (i=1), or `PEEK_FULL` (i=2) registers
  - `def peekraw(i: int) -> int`: simulate read from `ACCUM0_ADD` (i=0) or `ACCUM1_ADD` (i=1) registers
  - `def add(i: int, v: int)`: simulate write to `ACCUM0_ADD` (i=0) or `ACCUM1_ADD` (i=1) registers
  - `def base01(v: int)`: simulate write to `BASE_1AND0` registers
  - `def update()`: update result (automatically called internally)
  - `def save() -> InterpState`: save the interpolator state
  - `def restore(state: InterpState)`: restore the interpolator state

The `rp2040_interp.hardware` package contains an additional class for
interfacing with a hardware interpolator via the `pico-test-hw` firmare in the
`test/pico-test/` directory.

- class InterpHW(Interp)`: Proxy to a Hardware Interpolator
  - `def __init__(self, ..., port: Path = '/dev/ttyACM0', debug: bool = False)`: constructor
    - the first parameters are identical to `Interp`
    - path is the path to the tty device to the Raspberry Pi Pico
    - debug enables printing all serial commands and responses
  - avoid accessing the `accum`/`base`/`ctrl` arrays directly, since it will
    only affect the software simulation and not the hardware device, use the
    `set_...()` methods instead.
  - see methods of the `Interp` class
  - `def diff()`: return a diff between the hardware and software interpolator

## Testing

A suite of tests vectors generated from real hardware will be added to this
repo soon-ish.

The `tests/` folder contains a WIP test framework for generating and checking
test vectors.
