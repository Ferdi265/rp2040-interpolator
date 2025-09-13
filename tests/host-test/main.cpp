#include <cstdio>
#include <cstring>
#include <string_view>
#include <interp-test.hpp>

static char line_buffer[512];

std::string_view getline() {
    size_t length;

    while (true) {
        if (fgets(line_buffer, sizeof line_buffer, stdin) == NULL) {
            continue;
        }

        length = strlen(line_buffer);
        if (length > 0 && line_buffer[length - 1] == '\n') length--;
        break;
    }

    return std::string_view(line_buffer, length);
}

int main() {
    InterpSWTester tester;
    while (true) {
        std::string_view command = getline();
        std::string_view response = tester.parse_command(command);
        puts(response.data());
    }
}
