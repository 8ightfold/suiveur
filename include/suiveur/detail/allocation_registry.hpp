#ifndef MEMORY_TRACKER_ALLOCATION_REGISTRY_HPP
#define MEMORY_TRACKER_ALLOCATION_REGISTRY_HPP

#include <filesystem>
#include <map>
#include <vector>

#include "cttypeid.hpp"
#include "type_names.hpp"

namespace suiveur {
    namespace fs = std::filesystem;

    struct allocation_registry {
        struct data_location {
            std::size_t line = static_cast<std::size_t>(-1);
            fs::path filename;

            data_location() = default;
            data_location(const std::size_t line, const fs::path file) : line(line), filename(file) {}
        };

        struct allocation_key {
            void* data = nullptr;
            std::size_t type_hash;
            data_location allocation_point;
            data_location deletion_point;

            explicit allocation_key(const std::size_t type) : type_hash(type) {}

            allocation_key(void* addr, const std::size_t type, const std::size_t line, const fs::path file)
                    : data(addr), type_hash(type), allocation_point(line, file) {}
        };

        struct error_key {
            enum class error_type : std::size_t {
                previously_tracked,
                previously_deleted,
                type_pun,
                untracked,
            };

            allocation_key key;
            data_location loc;
            std::size_t type_hash;
            error_type err;

            error_key(allocation_key& key, data_location loc, error_type err)
                    : key(key), loc(std::move(loc)), type_hash(key.type_hash), err(err) {}

            error_key(allocation_key& key, data_location loc, error_type err, std::size_t type)
                    : key(key), loc(std::move(loc)), type_hash(type), err(err) {}
        };

        using deleted_iter = std::vector<allocation_key>::iterator;

        static void record_allocation(void*, std::size_t, std::size_t, fs::path);
        static bool record_deletion(void*, std::size_t, std::size_t, fs::path);
        static bool safe_deletion(void*, std::size_t, std::size_t, fs::path);

        static deleted_iter find_deleted(void*, std::size_t);
        static void print_errors();
        static void list_nonfreed();

        static allocation_registry& get();
        static void erase();
        static void pass();

        ~allocation_registry() {
#ifdef ENABLE_MEMORY_REGISTRY
            list_nonfreed();
            print_errors();
#endif
        }

    private:
        static inline allocation_registry* global_registry = nullptr;
        std::map<void*, allocation_key> registry_keys;
        std::vector<allocation_key> deleted_keys {};
        std::vector<error_key> errors {};
    };


    struct allocation_registry_handler {
        allocation_registry** global_registry_ptr = nullptr;
        ~allocation_registry_handler() {
            if(global_registry_ptr) {
                allocation_registry*& reg = *global_registry_ptr;
                delete reg;
            }
        }
    };


    template <typename T>
    T* register_allocation(T* ptr, const std::size_t line, const char* file) {
        using U = std::remove_cv_t<T>;
        auto& types = type_names::get().names;
        std::size_t type_hash = typeid(U).hash_code();

        if(types.count(type_hash) == 0) types[type_hash] = cttypeid<U>{}.name();
        allocation_registry::record_allocation(ptr, type_hash, line, file);
        return ptr;
    }

    template <typename T>
    T* register_deletion(T* ptr, const std::size_t line, const char* file) {
        using U = std::remove_cv_t<T>;
        auto& types = type_names::get().names;
        std::size_t type_hash = typeid(U).hash_code();

        if(types.count(type_hash) == 0) types[type_hash] = cttypeid<U>{}.name();
        allocation_registry::record_deletion(ptr, type_hash, line, file);
        return ptr;
    }

    template <typename T>
    T* do_safe_deletion(T* ptr, const std::size_t line, const char* file) {
        using U = std::remove_cv_t<T>;
        auto& types = type_names::get().names;
        std::size_t type_hash = typeid(U).hash_code();

        if(types.count(type_hash) == 0) types[type_hash] = cttypeid<U>{}.name();
        auto it = allocation_registry::find_deleted(ptr, type_hash);

        bool should_delete = allocation_registry::safe_deletion(ptr, type_hash, line, file);
        if(should_delete) delete ptr;
        return ptr;
    }
}

#ifdef ENABLE_MEMORY_REGISTRY
#   define REGISTER_ALLOC(ptr)      suiveur::register_allocation(ptr, __LINE__, __FILE__)
#   define REGISTER_DELETE(ptr)     suiveur::register_deletion(ptr, __LINE__, __FILE__)
#   define SAFE_DELETE(ptr)         suiveur::do_safe_deletion(ptr, __LINE__, __FILE__)
#   define RESET_REGISTRY()         suiveur::allocation_registry::erase()
#   define PASS_REGISTRY()          suiveur::allocation_registry::pass()
#else
#   define REGISTER_ALLOC(ptr)      (ptr)
#   define REGISTER_DELETE(ptr)     (ptr)
#   define SAFE_DELETE(ptr)         (delete ptr)
#   define RESET_REGISTRY()         void(0)
#   define PASS_REGISTRY()          void(0)
#endif


#endif //MEMORY_TRACKER_ALLOCATION_REGISTRY_HPP
