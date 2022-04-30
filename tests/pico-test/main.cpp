#include <cstdio>
#include <string_view>
#include "pico/stdio_usb.h"
#include "interp-test.h"

#ifndef INTERP_TYPE
#define INTERP_TYPE Interp
#endif

static char line_buffer[512];

std::string_view getline() {
    size_t i = 0;
    size_t length = sizeof line_buffer;

    while (true) {
        int c = getchar_timeout_us(1000);
        if (c == PICO_ERROR_TIMEOUT) continue;
        if (c == '\n' || c == '\r') break;
        if ((c == '\b' || c == '\x7f') && i > 0) {
            i--;
            continue;
        }

        if (i < length) {
            line_buffer[i] = c;
            i++;
        }
    }

    return std::string_view(line_buffer, i);
}

int main() {
    stdio_usb_init();

    InterpTester<INTERP_TYPE> tester;
    while (true) {
        std::string_view command = getline();
        std::string_view response = tester.parse_command(command);
        puts(response.data());
    }
}
