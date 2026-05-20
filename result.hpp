// SPDX-License-Identifier: MIT

#ifndef CPP_RESULT_RESULT_HPP
#define CPP_RESULT_RESULT_HPP

#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

// =================================================================================================
// Project files
// =================================================================================================

#include "include/optional.hpp"

#if !defined(__cplusplus) || __cplusplus < 201703L
#    error "Result<T, E> implementation requires C++17 or later."
#endif

// =================================================================================================
// Version
// =================================================================================================

#define CPP_RESULT_VERSION_MAJOR 0
#define CPP_RESULT_VERSION_MINOR 1
#define CPP_RESULT_VERSION_PATCH 0

#define CPP_RESULT_VERSION_STRING "0.1.0"

#define CPP_RESULT_VERSION_ENCODE(major, minor, patch) \
    (((major) * 10000) + ((minor) * 100) + (patch))

#define CPP_RESULT_VERSION                                                        \
    CPP_RESULT_VERSION_ENCODE(CPP_RESULT_VERSION_MAJOR, CPP_RESULT_VERSION_MINOR, \
                              CPP_RESULT_VERSION_PATCH)

#define RESULT_ERROR(_m)              \
    do {                              \
        std::cerr << _m << std::endl; \
        std::terminate();             \
    } while (0)

#ifdef RESULT_NAMESPACE
namespace lsr::result {
#endif

template <typename T, typename E, auto OkSentinel = tiny::UseDefaultValue,
          auto ErrSentinel = tiny::UseDefaultValue>
class Result;

// =================================================================================================
// Wrapper types
// =================================================================================================

namespace wrapper {

// =================================================================================================
// Ok
// =================================================================================================

template <typename T>
struct Ok {
    using value_type = T;
    explicit Ok(const T &v) : value(v) {}
    explicit Ok(T &&v) : value(std::move(v)) {}

    T value;
};

template <typename T>
struct Ok<T &> {
    using value_type = T &;
    explicit Ok(T &v) noexcept : value(std::addressof(v)) {}

    T *value;
};

template <>
struct Ok<void> {};

// =================================================================================================
// Err
// =================================================================================================

template <typename E>
struct Err {
    using value_type = E;
    explicit Err(const E &e) : value(e) {}
    explicit Err(E &&e) : value(std::move(e)) {}

    E value;
};

template <typename E>
struct Err<E &> {
    using value_type = E &;
    explicit Err(E &e) noexcept : value(std::addressof(e)) {}

    E *value;
};

template <>
struct Err<void> {};

}  // namespace wrapper

template <typename T>
[[maybe_unused]] static auto Ok(T &&ok) {
    using U = std::conditional_t<std::is_lvalue_reference_v<T>, T, std::decay_t<T>>;
    return wrapper::Ok<U>(std::forward<T>(ok));
}

template <typename E>
[[maybe_unused]] static auto Err(E &&err) {
    using U = std::conditional_t<std::is_lvalue_reference_v<E>, E, std::decay_t<E>>;
    return wrapper::Err<U>(std::forward<E>(err));
}

[[maybe_unused]] static auto Ok() { return wrapper::Ok<void>{}; }

[[maybe_unused]] static auto Err() { return wrapper::Err<void>{}; }

// =================================================================================================
// Helper functionality
// =================================================================================================

namespace detail {

template <typename X>
struct is_result : std::false_type {};

template <typename T, typename E, auto OS, auto ES>
struct is_result<Result<T, E, OS, ES>> : std::true_type {};

template <typename X>
struct destruct_result;

template <typename T, typename E, auto OS, auto ES>
struct destruct_result<Result<T, E, OS, ES>> {
    using ok_type [[maybe_unused]] = T;
    using err_type [[maybe_unused]] = E;
};

template <typename T>
using nonvoid_value_t [[maybe_unused]] = std::enable_if_t<!std::is_void_v<T>, T>;

template <typename T>
using nonvoid_ref_t [[maybe_unused]] = std::enable_if_t<!std::is_void_v<T>, T &>;

template <typename T>
using nonvoid_cref_t [[maybe_unused]] = std::enable_if_t<!std::is_void_v<T>, const T &>;

// =================================================================================================
// Stored type resolve
// =================================================================================================

template <typename T>
struct stored_type {
    using type = T;
};

template <typename T>
struct stored_type<T &> {
    using type = T *;
};

template <typename T>
using stored_type_t = typename stored_type<T>::type;

template <typename T>
[[maybe_unused]] inline constexpr bool is_ref_v = std::is_lvalue_reference_v<T>;

template <typename T>
[[maybe_unused]] static stored_type_t<T> store_value(T value) {
    if constexpr (std::is_lvalue_reference_v<T>) {
        return std::addressof(value);
    } else {
        return std::move(value);
    }
}

// =================================================================================================
// Reference handling
// =================================================================================================

template <typename T>
[[maybe_unused]] static T unwrap_stored(stored_type_t<T> &value) {
    if constexpr (std::is_lvalue_reference_v<T>) {
        return *value;
    } else {
        return std::move(value);
    }
}

template <typename T>
[[maybe_unused]] static std::add_lvalue_reference_t<std::remove_reference_t<T>> unwrap_stored_ref(
    stored_type_t<T> &value) {
    if constexpr (std::is_lvalue_reference_v<T>) {
        return *value;
    } else {
        return value;
    }
}

template <typename T>
[[maybe_unused]] static std::add_lvalue_reference_t<const std::remove_reference_t<T>>
unwrap_stored_cref(const stored_type_t<T> &value) {
    if constexpr (std::is_lvalue_reference_v<T>) {
        return *value;
    } else {
        return value;
    }
}

// =================================================================================================
// Internal type instance storage
// =================================================================================================

template <typename T, auto Sentinel>
class ok_optional_storage {
   protected:
    using public_type [[maybe_unused]] = T;
    using stored_type [[maybe_unused]] = stored_type_t<T>;

    tiny::optional<stored_type, Sentinel> m_ok;

    ok_optional_storage() = default;

    [[maybe_unused]] explicit ok_optional_storage(wrapper::Ok<T> ok) {
        if constexpr (std::is_lvalue_reference_v<T>) {
            m_ok = ok.value;  // T*
        } else {
            m_ok = std::move(ok.value);  // T
        }

        assert(m_ok.has_value() && "Ok value equals the configured empty sentinel.");
    }

    [[maybe_unused]] [[nodiscard]] bool has_ok() const noexcept { return m_ok.has_value(); }

    [[maybe_unused]] [[nodiscard]] decltype(auto) ok_ref() & {
        if constexpr (std::is_lvalue_reference_v<T>) {
            return **m_ok;  // T&
        } else {
            return (*m_ok);  // T&
        }
    }

    [[maybe_unused]] [[nodiscard]] decltype(auto) ok_ref() const & {
        if constexpr (std::is_lvalue_reference_v<T>) {
            return **m_ok;  // T&
        } else {
            return (*m_ok);  // const T&
        }
    }

    [[maybe_unused]] [[nodiscard]] decltype(auto) ok_take() && {
        if constexpr (std::is_lvalue_reference_v<T>) {
            return **m_ok;  // T&
        } else {
            return std::move(*m_ok);  // T
        }
    }
};

template <typename E, auto Sentinel>
class err_optional_storage {
   protected:
    using public_type [[maybe_unused]] = E;
    using stored_type [[maybe_unused]] = stored_type_t<E>;

    tiny::optional<stored_type, Sentinel> m_err;

    err_optional_storage() = default;

    [[maybe_unused]] explicit err_optional_storage(wrapper::Err<E> err) {
        if constexpr (std::is_lvalue_reference_v<E>) {
            m_err = err.value;  // E*
        } else {
            m_err = std::move(err.value);  // E
        }

        assert(m_err.has_value() && "Err value equals the configured empty sentinel.");
    }

    [[maybe_unused]] [[nodiscard]] bool has_err() const noexcept { return m_err.has_value(); }

    [[maybe_unused]] [[nodiscard]] decltype(auto) err_ref() & {
        if constexpr (std::is_lvalue_reference_v<E>) {
            return **m_err;
        } else {
            return (*m_err);
        }
    }

    [[maybe_unused]] [[nodiscard]] decltype(auto) err_ref() const & {
        if constexpr (std::is_lvalue_reference_v<E>) {
            return **m_err;
        } else {
            return (*m_err);
        }
    }

    [[maybe_unused]] [[nodiscard]] decltype(auto) err_take() && {
        if constexpr (std::is_lvalue_reference_v<E>) {
            return **m_err;
        } else {
            return std::move(*m_err);
        }
    }
};

template <typename T>
[[maybe_unused]] inline constexpr bool is_result_ref_v = std::is_lvalue_reference_v<T>;

template <typename T>
using result_ref_base_t [[maybe_unused]] = std::remove_reference_t<T>;

// =================================================================================================
// Sentinel type values
// =================================================================================================

template <auto V>
[[maybe_unused]] inline constexpr bool is_default_sentinel_v =
    std::is_same_v<std::decay_t<decltype(V)>, tiny::UseDefaultType> && V == tiny::UseDefaultValue;

template <typename T, auto Sentinel>
[[maybe_unused]] inline constexpr bool sentinel_type_compatible_v =
    is_default_sentinel_v<Sentinel> || std::is_convertible_v<decltype(Sentinel), T>;

template <typename T, auto Sentinel>
[[maybe_unused]] inline constexpr bool sentinel_not_void_v =
    !std::is_void_v<T> || is_default_sentinel_v<Sentinel>;

// =================================================================================================
// Exception
// =================================================================================================

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
class bad_result_access : public std::exception {
   public:
    const char *what() const noexcept override { return "Bad Result access!"; }
};
#endif

}  // namespace detail

// =================================================================================================
// Result<T, E> where T and E are void - degenerate bool storage case
// =================================================================================================

template <>
class [[nodiscard]] Result<void, void> {
   public:
    using ok_type [[maybe_unused]] = void;
    using err_type [[maybe_unused]] = void;

   private:
    bool m_ok;

   public:
    Result(wrapper::Ok<void>) noexcept : m_ok{true} {}
    Result(wrapper::Err<void>) noexcept : m_ok{false} {}
    Result(const Result &) = default;
    Result(Result &&) noexcept = default;

    Result &operator=(const Result &) = default;
    Result &operator=(Result &&) noexcept = default;

    ~Result() = default;

    // =============================================================================================
    // member functions
    // =============================================================================================

    [[nodiscard]] bool is_ok() const noexcept { return m_ok; }

    [[nodiscard]] bool is_err() const noexcept { return !m_ok; }

    [[maybe_unused]] void unwrap() const {
        if (!is_ok())
            RESULT_ERROR("Tried to unwrap a result containing an error");
    }

    [[maybe_unused]] void unwrap_err() const {
        if (!is_err())
            RESULT_ERROR("Tried to unwrap an error containing a result");
    }

    [[maybe_unused]] void unwrap_unchecked() const {}

    [[maybe_unused]] void unwrap_err_unchecked() const {}

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
    [[maybe_unused]] decltype(auto) unwrap_or_throw() && {
        if (is_ok())
            return std::move(*this).unwrap_unchecked();

        throw detail::bad_result_access{};
    }
#endif

    [[maybe_unused]] void expect(const std::string &message) const {
        if (!is_ok())
            RESULT_ERROR(message);
    }

    template <typename Fn>
    [[maybe_unused]] auto map(Fn &&fn) && {
        using Ret = std::invoke_result_t<Fn>;

        if (is_ok()) {
            if constexpr (std::is_void_v<Ret>) {
                std::invoke(std::forward<Fn>(fn));
                return Result<void, void>(Ok());
            } else {
                return Result<Ret, void>(Ok(std::invoke(std::forward<Fn>(fn))));
            }
        }

        if constexpr (std::is_void_v<Ret>) {
            return Result<void, void>(Err());
        } else {
            return Result<Ret, void>(Err());
        }
    }

    template <typename ErrFn>
    [[maybe_unused]] auto map_err(ErrFn &&fn) && {
        using ErrRet = std::invoke_result_t<ErrFn>;

        if (is_err()) {
            if constexpr (std::is_void_v<ErrRet>) {
                std::invoke(std::forward<ErrFn>(fn));
                return Result<void, void>(Err());
            } else {
                return Result<void, ErrRet>(Err(std::invoke(std::forward<ErrFn>(fn))));
            }
        }

        if constexpr (std::is_void_v<ErrRet>) {
            return Result<void, void>(Ok());
        } else {
            return Result<void, ErrRet>(Ok());
        }
    }

    template <typename Fn, typename Ret>
    [[maybe_unused]] Ret map_or(Fn &&fn, Ret fallback) && {
        return is_ok() ? std::invoke(std::forward<Fn>(fn)) : std::move(fallback);
    }

    template <typename Fn, typename FnOther>
    [[maybe_unused]] auto map_or_else(Fn &&fn, FnOther &&other) && {
        return is_ok() ? std::invoke(std::forward<Fn>(fn))
                       : std::invoke(std::forward<FnOther>(other));
    }

    template <typename Fn>
    [[maybe_unused]] auto and_then(Fn &&fn) && {
        using Ret = std::invoke_result_t<Fn>;
        static_assert(detail::is_result<Ret>::value,
                      "and_then callback must return Result<U, void>.");
        static_assert(std::is_same_v<typename detail::destruct_result<Ret>::err_type, void>,
                      "and_then callback must preserve the error type void.");

        if (is_ok())
            return std::invoke(std::forward<Fn>(fn));

        return Ret(Err());
    }

    template <typename ErrFn>
    [[maybe_unused]] auto or_else(ErrFn &&fn) && {
        using ErrRet = std::invoke_result_t<ErrFn>;
        static_assert(detail::is_result<ErrRet>::value,
                      "or_else callback must return Result<void, U>.");
        static_assert(std::is_same_v<typename detail::destruct_result<ErrRet>::ok_type, void>,
                      "or_else callback must preserve the ok type void.");

        if (is_err())
            return std::invoke(std::forward<ErrFn>(fn));

        return ErrRet(Ok());
    }

    bool operator==(const wrapper::Ok<void> &) const { return is_ok(); }

    bool operator!=(const wrapper::Ok<void> &ok) const { return !(*this == ok); }

    bool operator==(const wrapper::Err<void> &) const { return is_err(); }

    bool operator!=(const wrapper::Err<void> &err) const { return !(*this == err); }

    bool operator==(const Result<void, void> &other) const { return m_ok == other.m_ok; }

    bool operator!=(const Result<void, void> &other) const { return !(*this == other); }
};

// =================================================================================================
// Result<T, E> where E is void - enabling niche optimization
// =================================================================================================

template <typename T, auto OkSentinel, auto ErrSentinel>
class [[nodiscard]] Result<T, void, OkSentinel, ErrSentinel>
    : private detail::ok_optional_storage<T, OkSentinel> {
    static_assert(!std::is_void_v<T>, "Use Result<void, void>.");
    static_assert(!std::is_rvalue_reference_v<T>, "Result<T&&, void> is not supported.");

    static_assert(detail::is_default_sentinel_v<ErrSentinel>,
                  "ErrSentinel is meaningless for E == void.");

    using storage = detail::ok_optional_storage<T, OkSentinel>;

   public:
    using ok_type [[maybe_unused]] = T;
    using err_type [[maybe_unused]] = void;

    Result(wrapper::Ok<T> ok) : storage(std::move(ok)) {}
    Result(wrapper::Err<void>) : storage() {}
    Result(const Result &) = default;
    Result(Result &&) noexcept = default;

    Result &operator=(const Result &) = default;
    Result &operator=(Result &&) noexcept = default;

    ~Result() = default;

    // =============================================================================================
    // member functions
    // =============================================================================================

    [[nodiscard]] bool is_ok() const noexcept { return storage::has_ok(); }

    [[nodiscard]] bool is_err() const noexcept { return !storage::has_ok(); }

    [[maybe_unused]] decltype(auto) unwrap_ref() & {
        if (!is_ok())
            RESULT_ERROR("Tried to unwrap_ref a result containing an error");

        return storage::ok_ref();
    }

    [[maybe_unused]] decltype(auto) unwrap_ref() const & {
        if (!is_ok())
            RESULT_ERROR("Tried to unwrap_ref a result containing an error");

        return storage::ok_ref();
    }

    [[maybe_unused]] decltype(auto) unwrap() && {
        if (!is_ok())
            RESULT_ERROR("Tried to unwrap a result containing an error");

        return std::move(*this).storage::ok_take();
    }

    [[maybe_unused]] decltype(auto) unwrap_unchecked() && {
        return std::move(*this).storage::ok_take();
    }

    [[maybe_unused]] void unwrap_err() const {
        if (!is_err())
            RESULT_ERROR("Tried to unwrap an error containing a result");
    }

    [[maybe_unused]] void unwrap_err_unchecked() const {}

    template <typename U = T, typename = std::enable_if_t<!std::is_lvalue_reference_v<U> &&
                                                          std::is_default_constructible_v<U>>>
    [[maybe_unused]] T unwrap_or_default() && {
        if (is_ok())
            return std::move(*this).storage::ok_take();

        return T{};
    }

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
    [[maybe_unused]] decltype(auto) unwrap_or_throw() && {
        if (is_ok())
            return std::move(*this).unwrap_unchecked();

        throw detail::bad_result_access{};
    }
#endif

    [[maybe_unused]] decltype(auto) expect(const std::string &message) && {
        if (!is_ok())
            RESULT_ERROR(message);

        return std::move(*this).storage::ok_take();
    }

    template <typename Fn>
    [[maybe_unused]] auto map(Fn &&fn) && {
        using Arg = decltype(std::move(*this).storage::ok_take());
        using Ret = std::invoke_result_t<Fn, Arg>;

        if (is_ok()) {
            if constexpr (std::is_void_v<Ret>) {
                std::invoke(std::forward<Fn>(fn), std::move(*this).storage::ok_take());
                return Result<void, void>(Ok());
            } else {
                return Result<Ret, void>(
                    Ok(std::invoke(std::forward<Fn>(fn), std::move(*this).storage::ok_take())));
            }
        }

        if constexpr (std::is_void_v<Ret>) {
            return Result<void, void>(Err());
        } else {
            return Result<Ret, void>(Err());
        }
    }

    template <typename ErrFn>
    [[maybe_unused]] auto map_err(ErrFn &&fn) && {
        using ErrRet = std::invoke_result_t<ErrFn>;

        if (is_err()) {
            if constexpr (std::is_void_v<ErrRet>) {
                std::invoke(std::forward<ErrFn>(fn));
                return Result<T, void, OkSentinel, ErrSentinel>(Err());
            } else {
                return Result<T, ErrRet>(Err(std::invoke(std::forward<ErrFn>(fn))));
            }
        }

        if constexpr (std::is_void_v<ErrRet>) {
            return Result<T, void, OkSentinel, ErrSentinel>(
                Ok(std::move(*this).storage::ok_take()));
        } else {
            return Result<T, ErrRet>(Ok(std::move(*this).storage::ok_take()));
        }
    }

    template <typename Fn, typename Ret>
    [[maybe_unused]] auto map_or(Fn &&fn, Ret fallback) && {
        if (is_ok())
            return std::invoke(std::forward<Fn>(fn), std::move(*this).storage::ok_take());

        return std::move(fallback);
    }

    template <typename Fn, typename FnOther>
    [[maybe_unused]] auto map_or_else(Fn &&fn, FnOther &&other) && {
        if (is_ok())
            return std::invoke(std::forward<Fn>(fn), std::move(*this).storage::ok_take());

        return std::invoke(std::forward<FnOther>(other));
    }

    template <typename Fn>
    [[maybe_unused]] auto and_then(Fn &&fn) && {
        using Arg = decltype(std::move(*this).storage::ok_take());
        using Ret = std::invoke_result_t<Fn, Arg>;

        static_assert(detail::is_result<Ret>::value,
                      "and_then callback must return Result<U, void>.");
        static_assert(std::is_same_v<typename detail::destruct_result<Ret>::err_type, void>,
                      "and_then callback must preserve the error type void.");

        if (is_ok())
            return std::invoke(std::forward<Fn>(fn), std::move(*this).storage::ok_take());

        return Ret(Err());
    }

    template <typename ErrFn>
    [[maybe_unused]] auto or_else(ErrFn &&fn) && {
        using ErrRet = std::invoke_result_t<ErrFn>;

        static_assert(detail::is_result<ErrRet>::value,
                      "or_else callback must return Result<T, U>.");
        static_assert(std::is_same_v<typename detail::destruct_result<ErrRet>::ok_type, T>,
                      "or_else callback must preserve the ok type T.");

        if (is_err())
            return std::invoke(std::forward<ErrFn>(fn));

        return ErrRet(Ok(std::move(*this).storage::ok_take()));
    }

    bool operator==(const wrapper::Ok<T> &ok) const {
        if (!is_ok())
            return false;

        if constexpr (std::is_lvalue_reference_v<T>) {
            return storage::ok_ref() == *ok.value;
        } else {
            return storage::ok_ref() == ok.value;
        }
    }

    bool operator!=(const wrapper::Ok<T> &ok) const {
        return !(*this == ok);  // keep as is
    }

    bool operator==(const wrapper::Err<void> &) const { return is_err(); }

    bool operator!=(const wrapper::Err<void> &err) const {
        return !(*this == err);  // keep as is
    }

    template <typename U, typename G, auto OS, auto ES>
    bool operator==(const Result<U, G, OS, ES> &other) const {
        if (is_ok() != other.is_ok())
            return false;

        if (is_ok())
            return unwrap_ref() == other.unwrap_ref();

        if constexpr (std::is_void_v<G>) {
            return true;
        } else {
            return false;
        }
    }

    template <typename U, typename G, auto OS, auto ES>
    bool operator!=(const Result<U, G, OS, ES> &other) const {
        return !(*this == other);  // keep as is
    }
};

// =================================================================================================
// Result<T, E> where T is void - enabling niche optimization
// =================================================================================================

template <typename E, auto OkSentinel, auto ErrSentinel>
class [[nodiscard]] Result<void, E, OkSentinel, ErrSentinel>
    : private detail::err_optional_storage<E, ErrSentinel> {
    static_assert(!std::is_void_v<E>, "Use Result<void, void>.");
    static_assert(!std::is_rvalue_reference_v<E>, "Result<void, E&&> is not supported.");
    static_assert(detail::is_default_sentinel_v<OkSentinel>,
                  "OkSentinel is meaningless for T == void.");

    using storage = detail::err_optional_storage<E, ErrSentinel>;

   public:
    using ok_type [[maybe_unused]] = void;
    using err_type [[maybe_unused]] = E;

    Result(wrapper::Ok<void>) : storage() {}
    Result(wrapper::Err<E> err) : storage(std::move(err)) {}
    Result(const Result &) = default;
    Result(Result &&) noexcept = default;

    Result &operator=(const Result &) = default;
    Result &operator=(Result &&) noexcept = default;

    ~Result() = default;

    // =============================================================================================
    // member functions
    // =============================================================================================

    [[nodiscard]] bool is_ok() const noexcept { return !storage::has_err(); }

    [[nodiscard]] bool is_err() const noexcept { return storage::has_err(); }

    [[maybe_unused]] void unwrap() const {
        if (!is_ok())
            RESULT_ERROR("Tried to unwrap a result containing an error");
    }

    [[maybe_unused]] void unwrap_unchecked() const {}

    [[maybe_unused]] decltype(auto) unwrap_err_ref() & {
        if (!is_err())
            RESULT_ERROR("Tried to unwrap_err_ref an error containing a result");

        return storage::err_ref();
    }

    [[maybe_unused]] decltype(auto) unwrap_err_ref() const & {
        if (!is_err())
            RESULT_ERROR("Tried to unwrap_err_ref an error containing a result");

        return storage::err_ref();
    }

    [[maybe_unused]] decltype(auto) unwrap_err() && {
        if (!is_err())
            RESULT_ERROR("Tried to unwrap_err an ok result");

        return std::move(*this).storage::err_take();
    }

    [[maybe_unused]] decltype(auto) unwrap_err_unchecked() && {
        return std::move(*this).storage::err_take();
    }

    [[maybe_unused]] void expect(const std::string &message) const {
        if (!is_ok())
            RESULT_ERROR(message);
    }

    template <typename Fn>
    [[maybe_unused]] auto map(Fn &&fn) && {
        using Ret = std::invoke_result_t<Fn>;

        if (is_ok()) {
            if constexpr (std::is_void_v<Ret>) {
                std::invoke(std::forward<Fn>(fn));
                return Result<void, E, OkSentinel, ErrSentinel>(Ok());
            } else {
                return Result<Ret, E>(Ok(std::invoke(std::forward<Fn>(fn))));
            }
        }

        if constexpr (std::is_void_v<Ret>) {
            return Result<void, E, OkSentinel, ErrSentinel>(
                Err(std::move(*this).unwrap_err_unchecked()));
        } else {
            return Result<Ret, E>(Err(std::move(*this).unwrap_err_unchecked()));
        }
    }

    template <typename ErrFn>
    [[maybe_unused]] auto map_err(ErrFn &&fn) && {
        using ErrArg = decltype(std::move(*this).storage::err_take());
        using ErrRet = std::invoke_result_t<ErrFn, ErrArg>;

        if (is_err()) {
            if constexpr (std::is_void_v<ErrRet>) {
                std::invoke(std::forward<ErrFn>(fn), std::move(*this).storage::err_take());
                return Result<void, void>(Err());
            } else {
                return Result<void, ErrRet>(Err(
                    std::invoke(std::forward<ErrFn>(fn), std::move(*this).storage::err_take())));
            }
        }

        if constexpr (std::is_void_v<ErrRet>) {
            return Result<void, void>(Ok());
        } else {
            return Result<void, ErrRet>(Ok());
        }
    }

    template <typename Fn, typename Ret>
    [[maybe_unused]] auto map_or(Fn &&fn, Ret fallback) && {
        if (is_ok())
            return std::invoke(std::forward<Fn>(fn));

        return std::move(fallback);
    }

    template <typename Fn, typename FnOther>
    [[maybe_unused]] auto map_or_else(Fn &&fn, FnOther &&other) && {
        if (is_ok())
            return std::invoke(std::forward<Fn>(fn));

        return std::invoke(std::forward<FnOther>(other), std::move(*this).storage::err_take());
    }

    template <typename Fn>
    [[maybe_unused]] auto and_then(Fn &&fn) && {
        using Ret = std::invoke_result_t<Fn>;

        static_assert(detail::is_result<Ret>::value, "and_then callback must return Result<U, E>.");
        static_assert(std::is_same_v<typename detail::destruct_result<Ret>::err_type, E>,
                      "and_then callback must preserve the error type E.");

        if (is_ok())
            return std::invoke(std::forward<Fn>(fn));

        return Ret(Err(std::move(*this).storage::err_take()));
    }

    template <typename ErrFn>
    [[maybe_unused]] auto or_else(ErrFn &&fn) && {
        using ErrArg = decltype(std::move(*this).storage::err_take());
        using ErrRet = std::invoke_result_t<ErrFn, ErrArg>;

        static_assert(detail::is_result<ErrRet>::value,
                      "or_else callback must return Result<void, U>.");
        static_assert(std::is_same_v<typename detail::destruct_result<ErrRet>::ok_type, void>,
                      "or_else callback must preserve the ok type void.");

        if (is_err())
            return std::invoke(std::forward<ErrFn>(fn), std::move(*this).storage::err_take());

        return ErrRet(Ok());
    }

    bool operator==(const wrapper::Ok<void> &) const { return is_ok(); }

    bool operator!=(const wrapper::Ok<void> &ok) const {
        return !(*this == ok);  // keep as is
    }

    bool operator==(const wrapper::Err<E> &err) const {
        if (!is_err())
            return false;

        if constexpr (std::is_lvalue_reference_v<E>) {
            return storage::err_ref() == *err.value;
        } else {
            return storage::err_ref() == err.value;
        }
    }

    bool operator!=(const wrapper::Err<E> &err) const {
        return !(*this == err);  // keep as is
    }

    template <typename U, typename G, auto OS, auto ES>
    bool operator==(const Result<U, G, OS, ES> &other) const {
        if (is_ok() != other.is_ok())
            return false;

        if (is_ok()) {
            if constexpr (std::is_void_v<U>) {
                return true;
            } else {
                return false;
            }
        }

        return unwrap_err_ref() == other.unwrap_err_ref();
    }

    template <typename U, typename G, auto OS, auto ES>
    bool operator!=(const Result<U, G, OS, ES> &other) const {
        return !(*this == other);  // keep as is
    }
};

// =================================================================================================
// Result<T, E> where neither T nor E are void
// =================================================================================================

template <typename T, typename E, auto OkSentinel, auto ErrSentinel>
class [[nodiscard]] Result {
    static_assert(!std::is_void_v<T>, "Use the Result<void, E> specialization.");
    static_assert(!std::is_void_v<E>, "Use the Result<T, void> specialization.");
    static_assert(!std::is_rvalue_reference_v<T>, "Result<T&&, E> is not supported.");
    static_assert(!std::is_rvalue_reference_v<E>, "Result<T, E&&> is not supported.");

    std::variant<wrapper::Ok<T>, wrapper::Err<E>> m_data;

    [[maybe_unused]] wrapper::Ok<T> &ok_state() { return std::get<0>(m_data); }

    const wrapper::Ok<T> &ok_state() const { return std::get<0>(m_data); }

    [[maybe_unused]] wrapper::Err<E> &err_state() { return std::get<1>(m_data); }

    const wrapper::Err<E> &err_state() const { return std::get<1>(m_data); }

    [[maybe_unused]] T ok_take() {
        if constexpr (std::is_lvalue_reference_v<T>) {
            return *ok_state().value;
        } else {
            return std::move(ok_state().value);
        }
    }

    [[maybe_unused]] E err_take() {
        if constexpr (std::is_lvalue_reference_v<E>) {
            return *err_state().value;
        } else {
            return std::move(err_state().value);
        }
    }

   public:
    using ok_type [[maybe_unused]] = T;
    using err_type [[maybe_unused]] = E;

    Result(wrapper::Ok<T> ok) : m_data{std::in_place_index<0>, std::move(ok)} {}

    Result(wrapper::Err<E> err) : m_data{std::in_place_index<1>, std::move(err)} {}

    Result(const Result &) = default;

    Result(Result &&) noexcept(std::is_nothrow_move_constructible_v<decltype(m_data)>) = default;

    Result &operator=(const Result &) = default;

    Result &operator=(Result &&) noexcept(std::is_nothrow_move_assignable_v<decltype(m_data)>) =
        default;

    ~Result() = default;

    // =============================================================================================
    // member functions
    // =============================================================================================

    [[nodiscard]] bool is_ok() const noexcept { return m_data.index() == 0; }

    [[nodiscard]] bool is_err() const noexcept { return m_data.index() == 1; }

    [[maybe_unused]] decltype(auto) unwrap_ref() & {
        if (!is_ok())
            RESULT_ERROR("Tried to unwrap_ref a result containing an error");

        if constexpr (std::is_lvalue_reference_v<T>) {
            return *ok_state().value;
        } else {
            return (ok_state().value);
        }
    }

    [[maybe_unused]] decltype(auto) unwrap_ref() const & {
        if (!is_ok())
            RESULT_ERROR("Tried to unwrap_ref a result containing an error");

        if constexpr (std::is_lvalue_reference_v<T>) {
            return *ok_state().value;
        } else {
            return static_cast<const T &>(ok_state().value);
        }
    }

    [[maybe_unused]] decltype(auto) unwrap_err_ref() & {
        if (!is_err())
            RESULT_ERROR("Tried to unwrap_err_ref an error containing a result");

        if constexpr (std::is_lvalue_reference_v<E>) {
            return *err_state().value;
        } else {
            return (err_state().value);
        }
    }

    [[maybe_unused]] decltype(auto) unwrap_err_ref() const & {
        if (!is_err())
            RESULT_ERROR("Tried to unwrap_err_ref an error containing a result");

        if constexpr (std::is_lvalue_reference_v<E>) {
            return *err_state().value;
        } else {
            return static_cast<const E &>(err_state().value);
        }
    }

    [[maybe_unused]] decltype(auto) unwrap() && {
        if (!is_ok())
            RESULT_ERROR("Tried to unwrap a result containing an error");

        return ok_take();
    }

    [[maybe_unused]] decltype(auto) unwrap_err() && {
        if (!is_err())
            RESULT_ERROR("Tried to unwrap an error containing a result");

        return err_take();
    }

    [[maybe_unused]] decltype(auto) unwrap_unchecked() && { return ok_take(); }

    [[maybe_unused]] decltype(auto) unwrap_err_unchecked() && { return err_take(); }

    [[maybe_unused]] T unwrap_or(T fallback) && {
        if (is_ok())
            return ok_take();

        return fallback;
    }

    template <typename U = T, typename = std::enable_if_t<!std::is_lvalue_reference_v<U> &&
                                                          std::is_default_constructible_v<U>>>
    [[maybe_unused]] T unwrap_or_default() && {
        if (is_ok())
            return ok_take();

        return T{};
    }

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
    [[maybe_unused]] decltype(auto) unwrap_or_throw() && {
        if (is_ok())
            return std::move(*this).unwrap_unchecked();

        static_assert(
            !std::is_lvalue_reference_v<E>,
            "unwrap_or_throw() is disabled for Result<T, E&> to avoid accidental slicing.");

        static_assert(std::is_base_of_v<std::exception, std::remove_reference_t<E>>,
                      "unwrap_or_throw() requires E to derive from std::exception.");

        throw std::move(*this).unwrap_err_unchecked();
    }
#endif

    [[maybe_unused]] decltype(auto) expect(const std::string &message) && {
        if (is_ok())
            return ok_take();

        RESULT_ERROR(message);
    }

    template <typename Fn>
    [[maybe_unused]] auto map(Fn &&fn) && {
        using Arg = decltype(std::declval<Result &>().ok_take());
        using Ret = std::invoke_result_t<Fn, Arg>;

        if (is_ok()) {
            if constexpr (std::is_void_v<Ret>) {
                std::invoke(std::forward<Fn>(fn), ok_take());
                return Result<void, E>(Ok());
            } else {
                return Result<Ret, E>(Ok(std::invoke(std::forward<Fn>(fn), ok_take())));
            }
        }

        if constexpr (std::is_void_v<Ret>) {
            return Result<void, E>(Err(err_take()));
        } else {
            return Result<Ret, E>(Err(err_take()));
        }
    }

    template <typename ErrFn>
    [[maybe_unused]] auto map_err(ErrFn &&fn) && {
        using ErrArg = decltype(std::declval<Result &>().err_take());
        using ErrRet = std::invoke_result_t<ErrFn, ErrArg>;

        if (is_err()) {
            if constexpr (std::is_void_v<ErrRet>) {
                std::invoke(std::forward<ErrFn>(fn), err_take());
                return Result<T, void>(Err());
            } else {
                return Result<T, ErrRet>(Err(std::invoke(std::forward<ErrFn>(fn), err_take())));
            }
        }

        if constexpr (std::is_void_v<ErrRet>) {
            return Result<T, void>(Ok(ok_take()));
        } else {
            return Result<T, ErrRet>(Ok(ok_take()));
        }
    }

    template <typename Fn, typename Ret>
    [[maybe_unused]] auto map_or(Fn &&fn, Ret fallback) && {
        if (is_ok())
            return std::invoke(std::forward<Fn>(fn), ok_take());

        return std::move(fallback);
    }

    template <typename Fn, typename FnOther>
    [[maybe_unused]] auto map_or_else(Fn &&fn, FnOther &&other) && {
        if (is_ok())
            return std::invoke(std::forward<Fn>(fn), ok_take());

        return std::invoke(std::forward<FnOther>(other), err_take());
    }

    template <typename Fn>
    [[maybe_unused]] auto and_then(Fn &&fn) && {
        using Arg = decltype(std::declval<Result &>().ok_take());
        using Ret = std::invoke_result_t<Fn, Arg>;

        static_assert(detail::is_result<Ret>::value, "and_then callback must return Result<U, E>.");

        static_assert(std::is_same_v<typename detail::destruct_result<Ret>::err_type, E>,
                      "and_then callback must preserve the error type E.");

        if (is_ok())
            return std::invoke(std::forward<Fn>(fn), ok_take());

        return Ret(Err(err_take()));
    }

    template <typename ErrFn>
    [[maybe_unused]] auto or_else(ErrFn &&fn) && {
        using ErrArg = decltype(std::declval<Result &>().err_take());
        using ErrRet = std::invoke_result_t<ErrFn, ErrArg>;

        static_assert(detail::is_result<ErrRet>::value,
                      "or_else callback must return Result<T, U>.");

        static_assert(std::is_same_v<typename detail::destruct_result<ErrRet>::ok_type, T>,
                      "or_else callback must preserve the ok type T.");

        if (is_err())
            return std::invoke(std::forward<ErrFn>(fn), err_take());

        return ErrRet(Ok(ok_take()));
    }

    bool operator==(const wrapper::Ok<T> &ok) const {
        if (!is_ok())
            return false;

        if constexpr (std::is_lvalue_reference_v<T>) {
            return unwrap_ref() == *ok.value;
        } else {
            return unwrap_ref() == ok.value;
        }
    }

    bool operator!=(const wrapper::Ok<T> &ok) const {
        return !(*this == ok);  // keep as is
    }

    bool operator==(const wrapper::Err<E> &err) const {
        if (!is_err())
            return false;

        if constexpr (std::is_lvalue_reference_v<E>) {
            return unwrap_err_ref() == *err.value;
        } else {
            return unwrap_err_ref() == err.value;
        }
    }

    bool operator!=(const wrapper::Err<E> &err) const {
        return !(*this == err);  // keep as is
    }

    template <typename U, typename G, auto OS, auto ES>
    bool operator==(const Result<U, G, OS, ES> &other) const {
        if (is_ok() != other.is_ok())
            return false;

        if (is_ok())
            return unwrap_ref() == other.unwrap_ref();

        return unwrap_err_ref() == other.unwrap_err_ref();
    }

    template <typename U, typename G, auto OS, auto ES>
    bool operator!=(const Result<U, G, OS, ES> &other) const {
        return !(*this == other);  // keep as is
    }
};

#ifdef RESULT_NAMESPACE
}  // namespace lsr::result
#endif

#undef RESULT_ERROR

#endif  // CPP_RESULT_RESULT_HPP
