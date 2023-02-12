#include "type_names.hpp"

namespace suiveur {
    type_names& type_names::get() {
        static type_names names {};
        return names;
    }
}