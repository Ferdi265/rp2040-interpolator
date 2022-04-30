#ifndef YRLF_INTERP_UTIL_H_
#define YRLF_INTERP_UTIL_H_

#include <stddef.h>
#include <string.h>
#include <type_traits>

template <typename T, typename U>
constexpr T bit_cast(const U& u) noexcept {
    static_assert(sizeof(T) == sizeof(U), "sizes of T and U must be equal");

    T t;
    memcpy(&t, &u, sizeof (T));
    return t;
}

template <typename T>
struct zext_t {
    template <typename U>
    constexpr std::make_unsigned_t<T> operator()(U u) const {
        return (std::make_unsigned_t<T>)(std::make_unsigned_t<U>)u;
    }
};

template <typename T>
struct sext_t {
    template <typename U>
    constexpr std::make_signed_t<T> operator()(U u) const {
        return (std::make_signed_t<T>)(std::make_signed_t<U>)u;
    }
};

template <typename T>
constexpr zext_t<T> zext{};

template <typename T>
constexpr sext_t<T> sext{};

#if RP2040_INTERP_WITH_HARDWARE
template <typename T, size_t addr>
struct reg_proxy {
    using ptr_type = T*;
    using ref_type = T&;
    constexpr ref_type get() { return *(ptr_type)addr; }
    constexpr operator ref_type() { return get(); }
};
#endif

#endif
