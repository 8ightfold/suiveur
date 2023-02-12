#include "allocation_registry.hpp"

#include <algorithm>
#include <iostream>
#include <type_traits>

#include "ansi_color.hpp"
#include "load_file.hpp"
#include "pad_with.hpp"
#include "partition_data.hpp"

namespace suiveur {
    void allocation_registry::record_allocation(void* addr, const std::size_t type, const std::size_t line, const fs::path file) {
        auto& keys = get().registry_keys;
        if(keys.count(addr) > 0) {
            auto& errors = get().errors;
            auto& value = keys.at(addr);
            errors.emplace_back(value, data_location{ line, file }, error_key::error_type::previously_tracked);
        }
        keys.emplace(addr, allocation_key { addr, type, line, file });
    }

    bool allocation_registry::record_deletion(void* addr, const std::size_t type, const std::size_t line, const fs::path file) {
        auto& keys = get().registry_keys;
        auto& deleted = get().deleted_keys;
        bool return_value = false;

        if(auto it = keys.find(addr); it != keys.end()) {
            return_value = true;
            auto& iter_value = it->second;
            iter_value.deletion_point = data_location{ line, file };
            if(iter_value.type_hash != type) {
                auto& errors = get().errors;
                errors.emplace_back(iter_value, data_location{ line, file }, error_key::error_type::type_pun, type);
                return_value = false;
            }
            else {
                deleted.push_back(iter_value);
                keys.erase(addr);
            }
        }
        return return_value;
    }

    bool allocation_registry::safe_deletion(void* addr, const std::size_t type, const std::size_t line, const fs::path file) {
        auto& keys = get().registry_keys;
        auto& deleted = get().deleted_keys;

        if(auto it = keys.find(addr); it != keys.end()) {
            return allocation_registry::record_deletion(addr, type, line, file);
        }
        else {
            auto deleted_key = allocation_registry::find_deleted(addr, type);
            auto& errors = get().errors;
            if(deleted_key == deleted.end()) {
                allocation_key key { type };
                errors.emplace_back(key, data_location{ line, file }, error_key::error_type::untracked);
            }
            else {
                errors.emplace_back(*deleted_key, data_location{ line, file }, error_key::error_type::previously_deleted);
            }
            return false;
        }
    }

    allocation_registry::deleted_iter allocation_registry::find_deleted(void* addr, const std::size_t type) {
        auto& deleted = get().deleted_keys;
        return std::find_if(
                deleted.begin(), deleted.end(),
                [&](allocation_key& mem) -> bool { return (mem.data == addr) && (mem.type_hash == type); }
        );
    }

    allocation_registry& allocation_registry::get() {
        static allocation_registry_handler reg_handler {};
        allocation_registry*& reg = global_registry;
        if(not reg) reg = new allocation_registry {};
        reg_handler.global_registry_ptr = &reg;
        return *reg;
    }

    void allocation_registry::erase() {
        allocation_registry*& reg = global_registry;
        delete reg;
        reg = nullptr;
    }

    void allocation_registry::pass() {
        allocation_registry*& reg = global_registry;
        auto* new_reg = new allocation_registry {};
        new_reg->registry_keys = reg->registry_keys;
        reg->registry_keys.clear();
        delete reg;
        reg = new_reg;
    }

    void allocation_registry::print_errors() {
        using error_t = allocation_registry::error_key::error_type;
        auto& errors { get().errors };
        auto& types { type_names::get().names };
        std::map<fs::path, std::unique_ptr<std::string>> file_data;
        std::map<fs::path, std::vector<std::string_view>> file_lines;

        auto load_file = [&](const fs::path& filepath) {
            if(file_data.count(filepath) == 0) {
                file_data.emplace(filepath, std::make_unique<std::string>());
                suiveur::load_file(filepath, *file_data[filepath]);
                file_lines.emplace(filepath, partition_data(*file_data[filepath]));
            }
        };
        auto get_spaces = [](const std::string_view& sv) -> std::size_t {
            std::size_t idx = 0;
            for(auto c : sv) {
                if(c != ' ') return idx;
                ++idx;
            }
            return 0;
        };
        if(errors.empty()) return;

        std::cout << ansi::red << "allocation errors found: \n" << ansi::reset;

        for(auto& error : errors) {
            const auto err_path { error.loc.filename };
            const auto path_name { err_path.string() };
            const auto on_line = error.loc.line - 1;
            auto padding { pad_with(on_line + 1) };

            const std::string print_path = (err_path.parent_path().filename() / err_path.filename()).string();

            fs::path decl_path;
            std::string decl_pad;
            std::size_t decl_line;
            std::size_t decl_spaces;

            load_file(err_path);
            const auto count_spaces { get_spaces(file_lines[err_path][on_line]) };

            switch(error.err) {
                case error_t::previously_deleted:
                {
                    decl_path = error.key.deletion_point.filename;
                    decl_line = error.key.deletion_point.line - 1;
                    decl_pad = std::string(padding.size() - pad_with(decl_line + 1).size(), ' ' );
                    load_file(decl_path);
                    decl_spaces = get_spaces(file_lines[decl_path][decl_line]);

                    std::cout << ansi::red << "error"
                              << ansi::reset << ": double deletion found\n";
                    std::cout << padding << " ---> " << print_path << ':' << on_line + 1 << ':' << '\n';
                    std::cout << padding << " | \n";
                    std::cout << decl_line + 1 << decl_pad << " | "
                              << ansi::blue << file_lines[decl_path][decl_line]
                              << ansi::reset << '\n';
                    std::cout << padding << " | "
                              << std::string(decl_spaces, ' ')
                              << ansi::blue << std::string(file_lines[decl_path][decl_line].size() - decl_spaces, '-')
                              << " initial deletion here\n" << ansi::reset;
                    std::cout << padding << " | \n";

                    std::cout << on_line + 1 << " | "
                              << ansi::red << file_lines[err_path][on_line]
                              << ansi::reset << '\n';
                    std::cout << padding << " | "
                              << std::string(count_spaces, ' ')
                              << ansi::red << std::string(file_lines[err_path][on_line].size() - count_spaces, '^')
                              << " second delete here\n" << ansi::reset;
                    std::cout << padding << " | \n";
                }
                    break;

                case error_t::previously_tracked:
                {
                    decl_path = error.key.allocation_point.filename;
                    decl_line = error.key.allocation_point.line - 1;
                    decl_pad = std::string( padding.size() - pad_with(decl_line + 1).size(), ' ' );
                    load_file(decl_path);
                    decl_spaces = get_spaces(file_lines[decl_path][decl_line]);

                    std::cout << ansi::red << "error"
                              << ansi::reset << ": overwrote tracking of previously tracked variable\n";
                    std::cout << padding << " ---> " << print_path << ':' << on_line + 1 << ':' << '\n';
                    std::cout << padding << " | \n";
                    std::cout << decl_line + 1 << decl_pad << " | "
                              << ansi::blue << file_lines[decl_path][decl_line]
                              << ansi::reset << '\n';
                    std::cout << padding << " | "
                              << std::string(decl_spaces, ' ')
                              << ansi::blue << std::string(file_lines[decl_path][decl_line].size() - decl_spaces, '-')
                              << " initial tracking here\n" << ansi::reset;
                    std::cout << padding << " | \n";

                    std::cout << on_line + 1 << " | "
                              << ansi::red << file_lines[err_path][on_line]
                              << ansi::reset << '\n';
                    std::cout << padding << " | "
                              << std::string(count_spaces, ' ')
                              << ansi::red << std::string(file_lines[err_path][on_line].size() - count_spaces, '^')
                              << " overwrote tracking here\n" << ansi::reset;
                    std::cout << padding << " | \n";
                }
                    break;

                case error_t::untracked:
                {
                    std::cout << ansi::red << "error"
                              << ansi::reset << ": attempted deletion of untracked pointer\n";
                    std::cout << padding << " ---> " << print_path << ':' << on_line + 1 << ':' << '\n';
                    std::cout << padding << " | \n";
                    std::cout << on_line + 1 << " | "
                              << ansi::red << file_lines[err_path][on_line]
                              << ansi::reset << '\n';
                    std::cout << padding << " | "
                              << std::string(count_spaces, ' ')
                              << ansi::red << std::string(file_lines[err_path][on_line].size() - count_spaces, '^')
                              << " here\n" << ansi::reset;
                    std::cout << padding << " | \n";
                }
                    break;

                case error_t::type_pun:
                {
                    decl_path = error.key.allocation_point.filename;
                    decl_line = error.key.allocation_point.line - 1;
                    decl_pad = std::string( padding.size() - pad_with(decl_line + 1).size(), ' ' );
                    load_file(decl_path);
                    decl_spaces = get_spaces(file_lines[decl_path][decl_line]);

                    std::cout << ansi::red << "error"
                              << ansi::reset << ": deleted pointer did not match allocated type\n";
                    std::cout << padding << " ---> " << print_path << ':' << on_line + 1 << ':' << '\n';
                    std::cout << padding << " | \n";
                    std::cout << decl_line + 1 << decl_pad << " | "
                              << ansi::blue << file_lines[decl_path][decl_line]
                              << ansi::reset << '\n';
                    std::cout << padding << " | "
                              << std::string(decl_spaces, ' ')
                              << ansi::blue << std::string(file_lines[decl_path][decl_line].size() - decl_spaces, '-')
                              << " allocated as type \"" << types[error.key.type_hash] << '"' << ansi::reset << '\n';
                    std::cout << padding << " | \n";

                    std::cout << on_line + 1 << " | "
                              << ansi::red << file_lines[err_path][on_line]
                              << ansi::reset << '\n';
                    std::cout << padding << " | "
                              << std::string(count_spaces, ' ')
                              << ansi::red << std::string(file_lines[err_path][on_line].size() - count_spaces, '^')
                              << " deleted as type \"" << types[error.type_hash] << '"' << ansi::reset << '\n';
                    std::cout << padding << " | \n";
                }
                    break;
            }

            std::cout << std::endl;
        }
    }

    void allocation_registry::list_nonfreed() {
        auto& keys = get().registry_keys;
        if(keys.empty()) return;
        std::size_t free_count = keys.size();

        std::cout << ansi::red << "error" << ansi::reset << ": " << free_count
                  << " unfreed pointer" << ((free_count == 1) ? "" : "s") << " at:\n";
        std::cout << ansi::red;
        for(auto& [key, value] : keys) {
            auto& file = value.allocation_point.filename;
            const auto line = value.allocation_point.line;
            const std::string print_path = (file.parent_path().filename() / file.filename()).string();
            std::cout << "---> " << print_path << ':' << line << '\n';
        }
        std::cout << ansi::reset << std::endl;
    }
}
