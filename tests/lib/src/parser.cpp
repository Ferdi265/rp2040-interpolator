#include <cstddef>
#include <charconv>
#include <limits>
#include "interp-test.h"

struct line_parser {
    std::string_view line;

    line_parser(std::string_view line) : line(line) {}

    bool peek_word(std::string_view& word) {
        size_t end = line.find(' ');

        if (end == line.npos) {
            end = line.size();
        }

        word = line.substr(0, end);
        return word.size() != 0;
    }

    bool has_word() {
        std::string_view word;
        return peek_word(word);
    }

    void pop_word() {
        size_t end = line.find(' ');

        if (end == line.npos) {
            end = line.size();
        }

        line = line.substr(end);
    }

    bool parse_word(std::string_view& word) {
        if (!peek_word(word)) return false;

        line = line.substr(word.size());
        return true;
    }

    bool parse_reg(InterpReg& reg) {
        std::string_view word;
        if (!peek_word(word)) return false;

        if (word == "accum0" || word == "ACCUM0") {
            reg = InterpReg::ACCUM0;
        } else if (word == "accum1" || word == "ACCUM1") {
            reg = InterpReg::ACCUM1;
        } else if (word == "base0" || word == "BASE0") {
            reg = InterpReg::BASE0;
        } else if (word == "base1" || word == "BASE1") {
            reg = InterpReg::BASE1;
        } else if (word == "base2" || word == "BASE2") {
            reg = InterpReg::BASE2;
        } else if (word == "ctrl0" || word == "CTRL0") {
            reg = InterpReg::CTRL0;
        } else if (word == "ctrl1" || word == "CTRL1") {
            reg = InterpReg::CTRL1;
        } else if (word == "pop0" || word == "POP0") {
            reg = InterpReg::POP0;
        } else if (word == "pop1" || word == "POP1") {
            reg = InterpReg::POP1;
        } else if (word == "pop2" || word == "POP2") {
            reg = InterpReg::POP2;
        } else if (word == "peek0" || word == "PEEK0") {
            reg = InterpReg::PEEK0;
        } else if (word == "peek1" || word == "PEEK1") {
            reg = InterpReg::PEEK1;
        } else if (word == "peek2" || word == "PEEK2") {
            reg = InterpReg::PEEK2;
        } else if (word == "peekraw0" || word == "PEEKRAW0") {
            reg = InterpReg::PEEKRAW0;
        } else if (word == "peekraw1" || word == "PEEKRAW1") {
            reg = InterpReg::PEEKRAW1;
        } else if (word == "add0" || word == "ADD0") {
            reg = InterpReg::ADD0;
        } else if (word == "add1" || word == "ADD1") {
            reg = InterpReg::ADD1;
        } else if (word == "base01" || word == "BASE01") {
            reg = InterpReg::BASE01;
        } else {
            return false;
        }

        pop_word();
        return true;
    }

    template <typename T>
    bool peek_int(T& num) {
        std::string_view word;
        if (!peek_word(word)) return false;

        std::from_chars_result result;
        std::string_view hex_prefix = word.substr(0, 2);
        if (hex_prefix == "0x" || hex_prefix == "0X") {
            word = word.substr(2);
            result = std::from_chars(word.begin(), word.end(), num, 16);
        } else {
            result = std::from_chars(word.begin(), word.end(), num, 10);
        }

        if (result.ptr != word.end()) return false;
        return true;
    }

    template <typename T>
    bool parse_int(T& num) {
        if (!peek_int(num)) return false;
        pop_word();
        return true;
    }

    bool parse_interp_num(interp_num_t& n) {
        uint8_t num;
        if (!peek_int(num)) return false;
        if (num != 0 && num != 1) return false;
        n = num;

        pop_word();
        return true;
    }

    bool parse_interp_write_state(InterpState& state) {
        if (!parse_int(state.accum[0])) return false;
        if (!parse_int(state.accum[1])) return false;
        if (!parse_int(state.base[0])) return false;
        if (!parse_int(state.base[1])) return false;
        if (!parse_int(state.base[2])) return false;
        if (!parse_int(state.ctrl[0])) return false;
        if (!parse_int(state.ctrl[1])) return false;
        return true;
    }

    bool parse_interp_dump_state(InterpState& state) {
        if (!parse_interp_write_state(state)) return false;
        if (!parse_int(state.peek[0])) return false;
        if (!parse_int(state.peek[1])) return false;
        if (!parse_int(state.peek[2])) return false;
        if (!parse_int(state.peekraw[0])) return false;
        if (!parse_int(state.peekraw[1])) return false;
        return true;
    }
};

struct buf_writer {
    char * buf;
    size_t length;
    bool first;

    buf_writer(char * buf, size_t n) : buf(buf), length(n), first(false) {
        if (n > 0) buf[0] = 0;
    }

    template <size_t N>
    buf_writer(char (&buf)[N]) : buf(buf), length(N) {}

    bool write_str(const char * str, size_t n) {
        if (length <= n) return false;

        memcpy(buf, str, n);
        buf[n] = 0;

        buf += n;
        length -= n;
        return true;
    }

    template <size_t N>
    bool write_str(const char (&str)[N]) {
        return write_str(str, N);
    }

    bool write_field(const char * str, size_t n) {
        if (!first && !write_str(" ")) return false;
        if (!write_str(str, n)) return false;
        first = true;
        return true;
    }

    template <size_t N>
    bool write_field(const char (&str)[N]) {
        return write_field(str, N);
    }

    template <typename T>
    bool write_int_dec(T t) {
        // sign and null terminator
        char buf[std::numeric_limits<T>::digits10 + 1 + 1];

        std::to_chars_result result;
        result = std::to_chars(std::begin(buf), std::end(buf), t, 10);
        if (result.ptr != std::end(buf)) return false;

        return write_field(buf, result.ptr - buf);
    }

    template <typename T>
    bool write_int_hex(T t) {
        // hex prefix, sign, and null terminator
        char buf[2 + std::numeric_limits<T>::digits10 + 1 + 1];
        memcpy(buf, "0x", 2);

        std::to_chars_result result;
        result = std::to_chars(std::begin(buf) + 2, std::end(buf), t, 16);
        if (result.ptr == std::end(buf)) return false;

        return write_field(buf, result.ptr - buf);
    }

    bool write_interp_dump_state(const InterpState& state) {
        if (!write_int_hex(state.accum[0])) return false;
        if (!write_int_hex(state.accum[1])) return false;
        if (!write_int_hex(state.base[0])) return false;
        if (!write_int_hex(state.base[1])) return false;
        if (!write_int_hex(state.base[2])) return false;
        if (!write_int_hex(state.ctrl[0])) return false;
        if (!write_int_hex(state.ctrl[1])) return false;
        if (!write_int_hex(state.peek[0])) return false;
        if (!write_int_hex(state.peek[1])) return false;
        if (!write_int_hex(state.peek[2])) return false;
        if (!write_int_hex(state.peekraw[0])) return false;
        if (!write_int_hex(state.peekraw[1])) return false;
        return true;
    }
};

bool operator==(const InterpState& state, const InterpState& expected) {
    if (state.accum[0] != expected.accum[0]) return false;
    if (state.accum[1] != expected.accum[1]) return false;
    if (state.base[0] != expected.base[0]) return false;
    if (state.base[1] != expected.base[1]) return false;
    if (state.base[2] != expected.base[2]) return false;
    if (state.ctrl[0] != expected.ctrl[0]) return false;
    if (state.ctrl[1] != expected.ctrl[1]) return false;
    if (state.peek[0] != expected.peek[0]) return false;
    if (state.peek[1] != expected.peek[1]) return false;
    if (state.peek[2] != expected.peek[2]) return false;
    if (state.peekraw[0] != expected.peekraw[0]) return false;
    if (state.peekraw[1] != expected.peekraw[1]) return false;
    return true;
}

bool operator!=(const InterpState& state, const InterpState& expected) {
    return !(state == expected);
}

constexpr static std::string_view fail_str = "fatal 'failed to write result'";
static char result_buffer[512];

std::string_view format_data_reg(uint32_t value) {
    buf_writer writer(result_buffer);

    if (!writer.write_field("data")) return fail_str;
    if (!writer.write_int_hex(value)) return fail_str;

    return result_buffer;
}

std::string_view format_data_state(const InterpState& state) {
    buf_writer writer(result_buffer);

    if (!writer.write_field("data")) return fail_str;
    if (!writer.write_interp_dump_state(state)) return fail_str;

    return result_buffer;
}

std::string_view format_diff_reg(uint32_t value, uint32_t expected) {
    buf_writer writer(result_buffer);

    uint32_t diff = value ^ expected;

    if (!writer.write_field("diff")) return fail_str;
    if (!writer.write_int_hex(diff)) return fail_str;

    return result_buffer;
}

std::string_view format_diff_state(const InterpState& state, const InterpState& expected) {
    buf_writer writer(result_buffer);

    InterpState diff = state;
    diff.accum[0] ^= expected.accum[0];
    diff.accum[1] ^= expected.accum[1];
    diff.base[0] ^= expected.base[0];
    diff.base[1] ^= expected.base[1];
    diff.base[2] ^= expected.base[2];
    diff.ctrl[0] ^= expected.ctrl[0];
    diff.ctrl[1] ^= expected.ctrl[1];
    diff.peek[0] ^= expected.peek[0];
    diff.peek[1] ^= expected.peek[1];
    diff.peek[2] ^= expected.peek[2];
    diff.peekraw[0] ^= expected.peekraw[0];
    diff.peekraw[1] ^= expected.peekraw[1];

    if (!writer.write_field("diff")) return fail_str;
    if (!writer.write_interp_dump_state(diff)) return fail_str;

    return result_buffer;
}

#define OK "ok"
#define SYNTAX_ERROR(msg) ("syntax '" msg "'")

std::string_view InterpTesterBase::parse_command(std::string_view cmdline) {
    line_parser parser(cmdline);

    std::string_view cmd;
    interp_num_t n;
    if (!parser.parse_word(cmd)) return SYNTAX_ERROR("expected command");
    if (!parser.parse_interp_num(n)) return SYNTAX_ERROR("expected interp_num");

    InterpState state;
    InterpState read_state;
    InterpReg reg;
    uint32_t value;
    uint32_t read_value;

    if (cmd == "state") {
        if (!parser.parse_interp_write_state(state)) return SYNTAX_ERROR("expected interp write state");
        write_state(n, state);
    } else if (cmd == "dump") {
        if (!parser.has_word()) {
            dump_state(n, read_state);
            return format_data_state(read_state);
        } else {
            if (!parser.parse_interp_dump_state(state)) return SYNTAX_ERROR("expected interp dump state");
            dump_state(n, read_state);
            if (state != read_state) return format_diff_state(state, read_state);
        }
    } else if (cmd == "write") {
        if (!parser.parse_reg(reg)) return SYNTAX_ERROR("expected register");
        if (!parser.parse_int(value)) return SYNTAX_ERROR("expected register value");
        write_reg(n, reg, value);
    } else if (cmd == "read") {
        if (!parser.parse_reg(reg)) return SYNTAX_ERROR("expected register");
        if (!parser.has_word()) {
            read_reg(n, reg, read_value);
            return format_data_reg(read_value);
        } else {
            if (!parser.parse_int(value)) return SYNTAX_ERROR("expected register value");
            read_reg(n, reg, read_value);
            if (value != read_value) return format_diff_reg(value, read_value);
        }
    } else {
        return SYNTAX_ERROR("invalid command");
    }

    return OK;
}
