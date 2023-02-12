#include "partition_data.hpp"
#include <string_view>

namespace suiveur {
    std::vector<std::string_view> partition_data(const std::string& data) {
        std::size_t pos = 0, off;
        std::vector<std::string_view> views;
        const char* under = data.data();

        while((off = data.find('\n', pos)) != std::string::npos) {
            const char* begin = under + pos;
            std::size_t distance = off - pos;
            views.emplace_back(begin, distance);
            pos = off + 1;
        }
        views.emplace_back(under + pos, data.size() - pos);
        return views;
    }
}
