# `rp2040-interpolator`

Software Emulation Library for the RP2040 Interpolator peripheral.

## The Interpolator

The Interpolator is a fun little peripheral integrated into the SIO block in
the RP2040. It's only accessible from the CPU, but can do some useful
calculations when you want to calculate indices into tables, textures,
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
  RESULT1 -->|1 /   +--------+    |  |----+->| Right Shift |-->| Mask |--+--| Sign Ext |-->|1 \  |  |_/           | | |                  |       |_/                      
             |_/      ACCUM1 ---->|1 /       +-------------+   +------+  |  +----------+   |  |--+       +------+ | | |  /---------------/                                
                                  |_/                                    \---------------->|0 /  \------>| RAW0 |-----+--------------------------------\                  
                    +--------+                                                             |_/           +------+ | |    |                             v     +---------+  
                    |  BASE2 |----------------------------------------------------------------------------------------------------------------------- ADD -->| RESULT2 |  
                    +--------+     _                                                       | \           +------+ | |    |                       | \   ^     +---------+  
              _                   | \                                    /---------------->|0 \  /------>| RAW1 |-----+------------------------->|0 \  |                  
             | \      ACCUM0 ---->|1 \       +-------------+   +------+  |  +----------+   |  |--+   _   +------+ | | |  |                       |  |--/                  
  RESULT1 -->|0 \   +--------+    |  |----+->| Right Shift |-->| Mask |--+--| Sign Ext |-->|1 /  |  | \           | | |  |         _        0 -->|1 /                     
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

## C++ Library

The `CMakeLists.txt` defines a header-only library `rp2040-interp` with main
header `include/interp.h`.

TODO: more documentation

## Python Library

The `python/` folder contains the python package `rp2040_interp`.

TODO: more documentation

## Testing

A suite of tests vectors generated from real hardware will be added to this
repo soon-ish.
