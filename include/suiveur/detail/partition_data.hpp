#ifndef COMPILER_PARTITION_DATA_HPP
#define COMPILER_PARTITION_DATA_HPP

#include <string>
#include <vector>

namespace suiveur {
    std::vector<std::string_view> partition_data(const std::string& data);
}

#endif //COMPILER_PARTITION_DATA_HPP
