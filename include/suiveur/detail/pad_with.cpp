#include "pad_with.hpp"

namespace suiveur {
    std::string pad_with(std::size_t value) {
        std::size_t width = 0;
        do {
            value /= 10;
            ++width;
        } while(value > 0);

        return std::string(width, ' ');
    }
}
