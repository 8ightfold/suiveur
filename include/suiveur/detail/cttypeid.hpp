#ifndef MEMORY_TRACKER_CTTYPEID_HPP
#define MEMORY_TRACKER_CTTYPEID_HPP


#include <array>
#include <string>
#include <string_view>
#include <type_traits>

namespace suiveur {
    template <std::size_t...NN>
    constexpr auto substr_to_array(std::string_view str, std::index_sequence<NN...>)
    {
        return std::array{ str[NN]..., '\0' };
    };

#if defined(__clang__)
#   define SURVEUR_PRETTY_FUNCTION __PRETTY_FUNCTION__
#   define SURVEUR_PRETTY_FUNCTION_START "[T = "
#   define SURVEUR_PRETTY_FUNCTION_END "]\0"
#elif defined(__GNUC__) && defined(__GNUG__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
#   define SURVEUR_PRETTY_FUNCTION __PRETTY_FUNCTION__
#   define SURVEUR_PRETTY_FUNCTION_START "[with T = "
#   define SURVEUR_PRETTY_FUNCTION_END "]\0"
#elif defined(_MSC_VER)
#   define SURVEUR_PRETTY_FUNCTION __FUNCSIG__
#   define SURVEUR_PRETTY_FUNCTION_START "pretty_function_array<"
#   define SURVEUR_PRETTY_FUNCTION_END ">(void)\0"
#else
#endif

    template <typename T>
    constexpr auto pretty_function_array()
    {
        constexpr std::string_view name = std::string_view{ SURVEUR_PRETTY_FUNCTION };
        constexpr std::string_view prefix = { SURVEUR_PRETTY_FUNCTION_START };
        constexpr std::string_view suffix = { SURVEUR_PRETTY_FUNCTION_END };

        constexpr std::size_t start = name.find(prefix) + prefix.size();
        constexpr std::size_t end = name.rfind(suffix);

        static_assert(start < end, "Invalid arguments.");

        constexpr std::string_view pname = name.substr(start, (end - start));
        return substr_to_array(pname, std::make_index_sequence<pname.size()>{});
    };

    template <typename Ty>
    struct pretty_parse {
    protected: static inline constexpr auto pretty_name = pretty_function_array<Ty>();
    };

    template <typename Ty>
    struct cttypeid final : pretty_parse<Ty>
    {
        constexpr cttypeid() = default;
        std::string name() const { return this->name_of; }
        friend std::ostream& operator << (std::ostream& os, const cttypeid id) { return os << id.name(); }
    private:
        std::string name_of = { cttypeid<Ty>::pretty_name.data(), cttypeid<Ty>::pretty_name.size() - 1 };
    };
}

#endif //MEMORY_TRACKER_CTTYPEID_HPP
