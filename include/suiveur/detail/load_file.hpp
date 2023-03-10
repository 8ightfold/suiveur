#ifndef COMPILER_LOAD_FILE_HPP
#define COMPILER_LOAD_FILE_HPP

#include <exception>
#include <filesystem>
#include <string>
#include <utility>

namespace suiveur {
    namespace fs = std::filesystem;

    std::string narrow(const char* str);
    std::string narrow(const wchar_t* str);

    struct failed_opening : std::exception {
        [[nodiscard]] const char* what() const noexcept override { return local.c_str(); }
        explicit failed_opening(fs::path fp) : filepath(std::move(fp)), local(narrow(filepath.c_str())) {}

    private:
        fs::path filepath;
        std::string local;
    };

    void load_file(const fs::path& filepath, std::string& internal_file_data);
}

#endif //COMPILER_LOAD_FILE_HPP
