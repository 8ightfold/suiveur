#ifndef COMPILER_ANSI_COLOR_HPP
#define COMPILER_ANSI_COLOR_HPP

#include <iostream>
#include <string_view>

namespace suiveur::ansi {
    struct ansi_base {
        std::string_view color { "\u001b[0m" };

        friend std::ostream& operator<<(std::ostream& os, const ansi_base& c) {
            return os << c.color;
        }
    };

#ifndef DISABLE_ANSI_COLOR
    inline constexpr ansi_base reset {};
    inline constexpr ansi_base red { "\u001b[31;1m" };
    inline constexpr ansi_base green { "\u001b[32;1m" };
    inline constexpr ansi_base blue { "\u001b[34;1m" };
    inline constexpr ansi_base yellow { "\u001b[33;1m" };
    inline constexpr ansi_base cyan { "\u001b[36;1m" };
    inline constexpr ansi_base white { "\u001b[37;1m" };
#else
    inline constexpr ansi_base reset { "" };
    inline constexpr ansi_base red { "" };
    inline constexpr ansi_base green { "" };
    inline constexpr ansi_base blue { "" };
    inline constexpr ansi_base yellow { "" };
    inline constexpr ansi_base cyan { "" };
    inline constexpr ansi_base white { "" };
#endif
}

#endif //COMPILER_ANSI_COLOR_HPP
