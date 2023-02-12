#include "load_file.hpp"
#include <fstream>
#include <iterator>

namespace suiveur {
    template <typename Facet>
    struct deletable_facet : Facet
    {
        template <typename...TT>
        deletable_facet(TT&& ...tt) : Facet(std::forward<TT>(tt)...) {}
        ~deletable_facet() {}
    };

    std::string narrow(const char* str) {
        return { str };
    }

    std::string narrow(const wchar_t* str) {
        std::wstring source(str);
        std::wstring_convert<deletable_facet<std::codecvt<wchar_t, char, std::mbstate_t>>, wchar_t> convert;
        std::string dest = convert.to_bytes(source);
        return dest;
    }


    void load_file(const fs::path& filepath, std::string& internal_file_data) {
        std::ifstream is ( filepath, std::ios::binary );
        is.unsetf(std::ios::skipws);
        if(not is) throw failed_opening { filepath };

        std::streampos size;

        is.seekg(0, std::ios::end);
        size = is.tellg();
        is.seekg(0, std::ios::beg);

        std::istream_iterator<char> start(is), end;
        internal_file_data.reserve(size);
        internal_file_data.insert(internal_file_data.cbegin(), start, end);
    }
}
