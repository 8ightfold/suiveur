#ifndef MEMORY_TRACKER_TYPE_NAMES_HPP
#define MEMORY_TRACKER_TYPE_NAMES_HPP

#include <map>
#include <string>
#include <typeinfo>

namespace suiveur {
    struct type_names {
        static type_names& get();
        std::map<std::size_t, std::string> names;

    private:
        type_names() = default;
    };
}

#endif //MEMORY_TRACKER_TYPE_NAMES_HPP
