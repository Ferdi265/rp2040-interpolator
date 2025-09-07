from typing import override
from serial import Serial
from pathlib import Path
from .interp import Interp, InterpState, InterpGeneration

def _hex_values(values: list[int]) -> str:
    return " ".join(hex(v) for v in values)

class InterpHW(Interp):
    serial: Serial
    debug: bool

    def __init__(self, n: int = 0, generation: InterpGeneration = InterpGeneration.RP2040, port: Path = Path("/dev/ttyACM0"), debug: bool = False):
        super().__init__(n, generation)
        self.serial = Serial(str(port), 115200)
        self.debug = debug

    def _send_cmd_raw(self, cmd: str) -> tuple[str, list[int]]:
        if self.debug:
            print(f"<< {cmd}")
        self.serial.write(cmd.encode() + b"\n")
        line = self.serial.readline().decode().strip()
        if self.debug:
            print(f">> {line}")

        parts = line.split(" ", 1)
        word, rest = parts[0], ("" if len(parts) == 1 else parts[1])
        if word == "syntax":
            raise SyntaxError(rest)

        if rest == "":
            return word, []

        try:
            rest_parts = rest.split(" ")
            values = [int(part, 0) for part in rest_parts]
        except ValueError:
            raise ValueError(f"expected list of integers, got '{rest}'")

        return word, values

    def _send_cmd_ok(self, cmd: str):
        word, values = self._send_cmd_raw(cmd)
        if word != "ok" or len(values) != 0:
            raise ValueError(f"expected 'ok', got '{word} {_hex_values(values)}'")

    def _send_cmd_n(self, cmd: str, expected_cmd: str, n: int) -> list[int]:
        word, values = self._send_cmd_raw(cmd)
        if word != expected_cmd or len(values) != n:
            raise ValueError(f"expected 'data' with {n} value(s), got '{word} {rest}'")

        return values

    def _send_cmd_data(self, cmd: str) -> int:
        return self._send_cmd_n(cmd, "data", 1)[0]

    def _write_reg(self, reg: str, v: int):
        self._send_cmd_ok(f"write {self.n} {reg} {v:#x}")

    def _read_reg(self, reg: str) -> int:
        return self._send_cmd_data(f"read {self.n} {reg}")

    def _write_state(self, state: InterpState):
        self._send_cmd_ok(f"state {self.n} {state.accum[0]:#x} {state.accum[1]:#x} {state.base[0]:#x} {state.base[1]:#x} {state.base[2]:#x} {state.ctrl[0]:#x} {state.ctrl[1]:#x}")

    def _read_state(self) -> InterpState:
        values = self._send_cmd_n(f"dump {self.n}", "data", 12)
        return InterpState(
            accum = values[0:2],
            base = values[2:5],
            ctrl = values[5:7],
            peek = values[7:10],
            peekraw = values[10:12]
        )

    @override
    def set_accum(self, i: int, v: int):
        super().set_accum(i, v)
        self._write_reg(f"accum{i}", v)

    @override
    def set_base(self, i: int, v: int):
        super().set_base(i, v)
        self._write_reg(f"base{i}", v)

    @override
    def set_ctrl(self, i: int, v: int):
        super().set_ctrl(i, v)
        self._write_reg(f"ctrl{i}", v)

    @override
    def pop(self, i: int) -> int:
        super().pop(i)
        return self._read_reg(f"pop{i}")

    @override
    def peek(self, i: int) -> int:
        super().peek(i)
        return self._read_reg(f"peek{i}")

    @override
    def peekraw(self, i: int) -> int:
        super().peekraw(i)
        return self._read_reg(f"peekraw{i}")

    @override
    def add(self, i: int, v: int) -> int:
        super().add(i, v)
        return self._write_reg(f"add{i}", v)

    @override
    def base01(self, v: int) -> int:
        super().base01(i, v)
        return self._write_reg("base01", v)

    @override
    def save(self, sw: bool = False) -> InterpState:
        sw_state = super().save()
        if sw:
            return sw_state
        else:
            return self._read_state()

    @override
    def restore(self, state: InterpState):
        super().restore(state)
        self._write_state(state)

    @override
    def diff(self) -> InterpState:
        sw_state = self.save(sw = True)
        hw_state = self.save(sw = False)
        return sw_state ^ hw_state
