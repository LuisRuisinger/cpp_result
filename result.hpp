#ifndef CPP_RESULT_RESULT_HPP
#define CPP_RESULT_RESULT_HPP

#include <iostream>
#include <functional>
#include <type_traits>

#if defined(__cplusplus) && __cplusplus >= 201703L
    #define RESULT_MAYBE_UNUSED [[maybe_unused]]
    #define RESULT_NODISCARD    [[nodiscard]]

#elif defined(__GNUC__) || defined(__clang__)
    #define RESULT_MAYBE_UNUSED __attribute__((unused))
    #define RESULT_NODISCARD    __attribute__((warn_unused_result))

#elif defined(_MSC_VER)
    #define RESULT_MAYBE_UNUSED __pragma(warning(suppress: 4505))
    #define RESULT_NODISCARD    __pragma(warning(error: 6031))

#else
    #define RESULT_MAYBE_UNUSED
    #define RESULT_NODISCARD

#endif

#define RESULT_ERROR(_m) \
    do { std::cerr << _m << std::endl; std::terminate(); } while (0)

#ifdef RESULT_NAMESPACE
namespace Result {
#endif

// forward declaration
template <typename T, typename E>
struct Result;

namespace Utils {

// implementation of a replacement of C++17's std::void_t
template <typename ...>
struct make_void { typedef void type; };

template <typename ...Args>
using void_t = typename make_void<Args...>::type;





// comparable trait
template <typename L, typename R, typename _ = void>
struct is_eq_comparable : std::false_type {};

template <typename L, typename R>
struct is_eq_comparable<
    L, R, void_t<decltype(std::declval<L>() == std::declval<R>())>> : std::true_type {};

template <typename L, typename R, typename _ = void>
struct is_ne_comparable : std::false_type {};

template <typename L, typename R>
struct is_ne_comparable<
    L, R, void_t<decltype(std::declval<L>() != std::declval<R>())>> : std::true_type {};

template <typename L, typename R, typename _ = void>
struct is_lt_comparable : std::false_type {};

template <typename L, typename R>
struct is_lt_comparable<
    L, R, void_t<decltype(std::declval<L>() < std::declval<R>())>> : std::true_type {};

template <typename L, typename R, typename _ = void>
struct is_gt_comparable : std::false_type {};

template <typename L, typename R>
struct is_gt_comparable<
    L, R, void_t<decltype(std::declval<L>() > std::declval<R>())>> : std::true_type {};

template <typename L, typename R, typename _ = void>
struct is_le_comparable : std::false_type {};

template <typename L, typename R>
struct is_le_comparable<
    L, R, void_t<decltype(std::declval<L>() <= std::declval<R>())>> : std::true_type {};

template <typename L, typename R, typename _ = void>
struct is_ge_comparable : std::false_type {};

template <typename L, typename R>
struct is_ge_comparable<
    L, R, void_t<decltype(std::declval<L>() >= std::declval<R>())>> : std::true_type {};





// implementation of a replacement of C++14's Utils::enable_if_t
template<bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;





// implementation of a replacement of C++14's std::is_convertible
namespace Details {

template <typename, typename _ = void>
struct is_returnable : std::false_type {};

template <typename T>
struct is_returnable<T, void_t<decltype(static_cast<T (*)()>(nullptr))>> : std::true_type {};

template <typename, typename, typename = void>
struct is_implicit_convertible : std::false_type {};

template <typename From, typename To>
struct is_implicit_convertible<From, To,
    void_t<decltype(std::declval<void(&)(To)>()(std::declval<From>()))>> : std::true_type {};

template <typename F, typename T>
struct is_convertible : std::integral_constant<
    bool, is_returnable<T>::value && is_implicit_convertible<F, T>::value> {};

template <>
struct is_convertible<void, void> : std::true_type {};

} // Details

template <typename F, typename T>
struct is_convertible : Details::is_convertible<F, T> {};





// implementation of a replacement of C++14's constexpr Utils::max
template <typename T, Utils::enable_if_t<Utils::is_eq_comparable<T, T>::value, bool> = true>
constexpr T max(T a, T b) {
    return a > b ? a : b;
}





// decomposition of function-like calls to extract the result
template <typename F, typename C = void>
struct result_of;

template <typename R, typename ...Args>
struct result_of<R (Args...)> { typedef R type; };

template <typename C, typename ...Args, typename R>
struct result_of<R (C::*)(Args...)> : result_of<R (Args...)> {};

template <typename C, typename ...Args, typename R>
struct result_of<R (C::*)(Args...) const> : result_of<R (Args...)> {};

template<typename R, typename ...Args>
struct result_of<R (*)(Args...)> : result_of<R (Args...)> {};

template <typename F>
struct result_of<
    F, Utils::void_t<decltype(&F::operator())>> : result_of<decltype(&F::operator())> {};

}





namespace Wrapper {

template <typename T>
struct Ok {
    explicit Ok (const T &t) : _t { t } {}
    explicit Ok (T &&t) : _t { std::move(t) } {}

    template <typename U, Utils::enable_if_t<Utils::is_eq_comparable<T, U>::value, bool> = true>
    bool operator==(const Ok<U> &other) {
        return _t == other._t;
    }

    template <typename U, Utils::enable_if_t<Utils::is_ne_comparable<T, U>::value, bool> = true>
    bool operator!=(const Ok<U> &other) {
        return _t != other._t;
    }

    template <typename U, Utils::enable_if_t<Utils::is_lt_comparable<T, U>::value, bool> = true>
    bool operator<(const Ok<U> &other) {
        return _t < other._t;
    }

    template <typename U, Utils::enable_if_t<Utils::is_gt_comparable<T, U>::value, bool> = true>
    bool operator>(const Ok<U> &other) {
        return _t > other._t;
    }

    template <typename U, Utils::enable_if_t<Utils::is_le_comparable<T, U>::value, bool> = true>
    bool operator<=(const Ok<U> &other) {
        return _t <= other._t;
    }

    template <typename U, Utils::enable_if_t<Utils::is_ge_comparable<T, U>::value, bool> = true>
    bool operator>=(const Ok<U> &other) {
        return _t >= other._t;
    }

    T _t;
};

template <>
struct Ok<void> {};

template <typename E>
struct Err {
    explicit Err (const E &e) : _e { e } {}
    explicit Err (E &&e) : _e { std::move(e) } {}

    template <typename U, Utils::enable_if_t<Utils::is_eq_comparable<E, U>::value, bool> = true>
    bool operator==(const Err<U> &other) {
        return _e == other._e;
    }

    template <typename U, Utils::enable_if_t<Utils::is_ne_comparable<E, U>::value, bool> = true>
    bool operator!=(const Err<U> &other) {
        return _e != other._e;
    }

    template <typename U, Utils::enable_if_t<Utils::is_lt_comparable<E, U>::value, bool> = true>
    bool operator<(const Err<U> &other) {
        return _e < other._e;
    }

    template <typename U, Utils::enable_if_t<Utils::is_gt_comparable<E, U>::value, bool> = true>
    bool operator>(const Err<U> &other) {
        return _e > other._e;
    }

    template <typename U, Utils::enable_if_t<Utils::is_le_comparable<E, U>::value, bool> = true>
    bool operator<=(const Err<U> &other) {
        return _e <= other._e;
    }

    template <typename U, Utils::enable_if_t<Utils::is_ge_comparable<E, U>::value, bool> = true>
    bool operator>=(const Err<U> &other) {
        return _e >= other._e;
    }

    E _e;
};

template <>
struct Err<void> {};

} // Wrapper





template <typename T>
RESULT_MAYBE_UNUSED Wrapper::Ok<typename std::decay<T>::type> Ok(T &&t) {
    return Wrapper::Ok<typename std::decay<T>::type>(std::forward<T>(t));
}

template <typename E>
RESULT_MAYBE_UNUSED Wrapper::Err<typename std::decay<E>::type> Err(E &&e) {
    return Wrapper::Err<typename std::decay<E>::type>(std::forward<E>(e));
}

RESULT_MAYBE_UNUSED Wrapper::Ok<void> Ok() {
    return Wrapper::Ok<void> {};
}

RESULT_MAYBE_UNUSED Wrapper::Err<void> Err() {
    return Wrapper::Err<void> {};
}





namespace Utils {

// QoL utility to check if some type is Result<T, E> and to decompose Result<T, E>
template <typename T> 
struct is_result : std::false_type {};

template <typename T, typename E>
struct is_result<Result<T, E>> : std::true_type {};

template <typename T>
struct destruct_result { typedef void type; };

template <typename T, typename E>
struct destruct_result<Result<T, E>> { typedef T Ok; typedef E Err; };





// storage container for an arbitrary result
template <typename T, typename E>
struct Storage {
    alignas(Utils::max(alignof(T), alignof(E)))
    std::array<std::uint8_t, Utils::max(sizeof(T), sizeof(E))> _storage = { 0 };
    std::size_t _type;

    RESULT_MAYBE_UNUSED explicit Storage(const Wrapper::Ok<T> &s)
        : _type { typeid(Wrapper::Ok<T>).hash_code() } { construct(s); }

    RESULT_MAYBE_UNUSED explicit Storage(const Wrapper::Err<E> &s)
        : _type { typeid(Wrapper::Err<E>).hash_code() } { construct(s); }

    RESULT_MAYBE_UNUSED explicit Storage(Wrapper::Ok<T> &&s)
        : _type { typeid(Wrapper::Ok<T>).hash_code() } { construct(std::move(s)); }

    RESULT_MAYBE_UNUSED explicit Storage(Wrapper::Err<E> &&s)
        : _type { typeid(Wrapper::Err<E>).hash_code() } { construct(std::move(s)); }

    void construct(Wrapper::Ok<T> ok) {
        (void) (*reinterpret_cast<T *>(this->_storage.data()) = std::move(ok._t));
    }

    void construct(Wrapper::Err<E> err) {
        (void) (*reinterpret_cast<E *>(this->_storage.data()) = std::move(err._e));
    }

    template <
        typename _,
        typename U = typename std::decay<_>::type,
        Utils::enable_if_t<!std::is_same<_, void>::value, bool> = true>
    auto get() const -> const U & {
        return *reinterpret_cast<const U *>(this->_storage.data());
    }

    template <
        typename _,
        typename U = typename std::decay<_>::type,
        Utils::enable_if_t<!std::is_same<_, void>::value, bool> = true>
    auto get() -> U & {
        return *reinterpret_cast<U *>(this->_storage.data());
    }

    template <
        typename _,
        typename U = typename std::decay<_>::type,
        Utils::enable_if_t<std::is_same<_, void>::value, bool> = true>
    auto get() const -> void {}

    template <typename S>
    RESULT_MAYBE_UNUSED RESULT_NODISCARD bool holds_alternative() const {
        return typeid(S).hash_code() == _type;
    }
};

template <typename E>
struct Storage<void, E> {
    alignas(alignof(E)) std::array<std::uint8_t, sizeof(E)> _storage = { 0 };;
    std::size_t _type;

    RESULT_MAYBE_UNUSED explicit Storage(const Wrapper::Ok<void> &t)
        : _type { typeid(Wrapper::Ok<void>).hash_code() } { construct(t); }

    RESULT_MAYBE_UNUSED explicit Storage(const Wrapper::Err<E> &s)
        : _type { typeid(Wrapper::Err<E>).hash_code() } { construct(s); }

    RESULT_MAYBE_UNUSED explicit Storage(Wrapper::Ok<void> &&t)
        : _type { typeid(Wrapper::Ok<void>).hash_code() } { construct(std::move(t)); }

    RESULT_MAYBE_UNUSED explicit Storage(Wrapper::Err<E> &&s)
        : _type { typeid(Wrapper::Err<E>).hash_code() } { construct(std::move(s)); }

    void construct(Wrapper::Ok<void> ok) {
        (void) ok;
    }

    void construct(Wrapper::Err<E> err) {
        (void) (*reinterpret_cast<E *>(this->_storage.data()) = std::move(err._e));
    }

    template <
        typename _,
        typename U = typename std::decay<_>::type,
        Utils::enable_if_t<!std::is_same<_, void>::value, bool> = true>
    auto get() const -> const U & {
        return *reinterpret_cast<const U *>(this->_storage.data());
    }

    template <
        typename _,
        typename U = typename std::decay<_>::type,
        Utils::enable_if_t<!std::is_same<_, void>::value, bool> = true>
    auto get() -> U & {
        return *reinterpret_cast<U *>(this->_storage.data());
    }

    template <
        typename _,
        typename U = typename std::decay<_>::type,
        Utils::enable_if_t<std::is_same<_, void>::value, bool> = true>
    auto get() const -> void {}

    template <typename S>
    RESULT_MAYBE_UNUSED RESULT_NODISCARD bool holds_alternative() const {
        return typeid(S).hash_code() == _type;
    }
};

template <typename T>
struct Storage<T, void> {
    alignas(alignof(T)) std::array<std::uint8_t, sizeof(T)> _storage = { 0 };
    std::size_t _type;

    RESULT_MAYBE_UNUSED explicit Storage(const Wrapper::Ok<T> &s)
        : _type { typeid(Wrapper::Ok<T>).hash_code() } { construct(s); }

    RESULT_MAYBE_UNUSED explicit Storage(const Wrapper::Err<void> &e)
        : _type { typeid(Wrapper::Err<void>).hash_code() } { construct(e); }

    RESULT_MAYBE_UNUSED explicit Storage(Wrapper::Ok<T> &&s)
        : _type { typeid(Wrapper::Ok<T>).hash_code() } { construct(std::move(s)); }

    RESULT_MAYBE_UNUSED explicit Storage(Wrapper::Err<void> &&e)
        : _type { typeid(Wrapper::Err<void>).hash_code() } { construct(std::move(e)); }

    void construct(Wrapper::Ok<T> ok) {
        (void) (*reinterpret_cast<T *>(this->_storage.data()) = std::move(ok._t) );
    }

    void construct(Wrapper::Err<void> err) {
        (void) err;
    }

    template <
        typename _,
        typename U = typename std::decay<_>::type,
        Utils::enable_if_t<!std::is_same<_, void>::value, bool> = true>
    auto get() const -> const U & {
        return *reinterpret_cast<const U *>(this->_storage.data());
    }

    template <
        typename _,
        typename U = typename std::decay<_>::type,
        Utils::enable_if_t<!std::is_same<_, void>::value, bool> = true>
    auto get() -> U & {
        return *reinterpret_cast<U *>(this->_storage.data());
    }

    template <
        typename _,
        typename U = typename std::decay<_>::type,
        Utils::enable_if_t<std::is_same<_, void>::value, bool> = true>
    auto get() const -> void {}

    template <typename S>
    RESULT_MAYBE_UNUSED RESULT_NODISCARD bool holds_alternative() const {
        return typeid(S).hash_code() == _type;
    }
};

template <>
struct Storage<void, void> {
    std::size_t _type;

    RESULT_MAYBE_UNUSED explicit Storage(const Wrapper::Ok<void> &t)
        : _type { typeid(Wrapper::Ok<void>).hash_code() } { std::ignore = t; }

    RESULT_MAYBE_UNUSED explicit Storage(const Wrapper::Err<void> &e)
        : _type { typeid(Wrapper::Err<void>).hash_code() } { std::ignore = e; }

    RESULT_MAYBE_UNUSED explicit Storage(Wrapper::Ok<void> &&t)
        : _type { typeid(Wrapper::Ok<void>).hash_code() } { std::ignore = t; }

    RESULT_MAYBE_UNUSED explicit Storage(Wrapper::Err<void> &&e)
        : _type { typeid(Wrapper::Err<void>).hash_code() } { std::ignore = e; }

    RESULT_MAYBE_UNUSED void construct(Wrapper::Ok<void> ok) {
        (void) ok;
    }

    RESULT_MAYBE_UNUSED void construct(Wrapper::Err<void> err) {
        (void) err;
    }

    template <typename _, typename U = std::decay<_>>
    void get() {}

    template <typename S>
    RESULT_MAYBE_UNUSED RESULT_NODISCARD bool holds_alternative() const {
        return typeid(S).hash_code() == _type;
    }
};

} // Utils





namespace Wrapper {
    template <typename S>
    struct Get;

    template <>
    struct Get<Ok<void>> {

        template <typename _>
        static Ok<void> get(const Result<void, _> &r) {
            std::ignore = r;
            return Ok<void> {};
        }  
    };

    template <typename T>
    struct Get<Ok<T>> {

        template <typename _>
        static Ok<T> get(const Result<T, _> &r) {
            auto ok = r.storage().template get<T>();
            return Ok<T>(std::move(ok));
        }
    };

    template <>
    struct Get<Err<void>> {

        template <typename _>
        static Err<void> get(const Result<_, void> &r) {
            std::ignore = r;
            return Err<void> {};
        }
    };

    template <typename E>
    struct Get<Err<E>> {

        template <typename _>
        static Err<E> get(const Result<_, E> &r) {
            auto err = r.storage().template get<E>();
            return Err<E>(std::move(err));
        }
    };
} // Wrapper




/**
 * Implementation namespace of transformer functions
 */
namespace Impl {

// map
template <typename T> struct Map;

template<typename R, typename C, typename ...Args>
struct Map<R (C::*)(Args...) const> : public Map<R (Args...)> {};

template<typename R, typename C, typename Args>
struct Map<R (C::*)(Args...)> : public Map<R (Args...)> {};

template <typename R, typename Arg>
struct Map<R (Arg)> {
    template <
        typename T, typename E, typename F,
        Utils::enable_if_t<Utils::is_convertible<T, Arg>::value, bool> = true>
    RESULT_MAYBE_UNUSED
    static Result<R, E> map(const Result<T, E> &r RESULT_MAYBE_UNUSED, F fun) {
        auto ok = r.storage().template get<T>();
        auto ret = fun(std::move(ok));
        return Ok(std::move(ret));
    }
};

template <typename Arg>
struct Map<void (Arg)> {
    template <
        typename T, typename E, typename F,
        Utils::enable_if_t<Utils::is_convertible<T, Arg>::value, bool> = true>
    RESULT_MAYBE_UNUSED
    static Result<void, E> map(const Result<T, E> &r RESULT_MAYBE_UNUSED, F fun) {
        auto ok = r.storage().template get<T>();
        fun(std::move(ok));
        return Wrapper::Ok<void>();
    }
};

template <typename R>
struct Map<R (void)> {
    template <typename T, typename E, typename F>
    RESULT_MAYBE_UNUSED
    static Result<R, E> map(const Result<T, E> &r RESULT_MAYBE_UNUSED, F fun) {
        auto ret = fun();
        return Ok(std::move(ret));
    }
};

template <>
struct Map<void (void)> {

    template <typename T,typename E, typename F>
    RESULT_MAYBE_UNUSED
    static Result<void, E> map(const Result<T, E> &r RESULT_MAYBE_UNUSED, F fun) {
        fun();
        return Wrapper::Ok<void>();
    }
};





// map err
template <typename T> struct MapErr;

template<typename R, typename C, typename ...Args>
struct MapErr<R (C::*)(Args...) const> : public MapErr<R (Args...)> {};

template<typename R, typename C, typename ...Args>
struct MapErr<R (C::*)(Args...)> : public MapErr<R (Args...)> {};

template <typename R, typename Arg>
struct MapErr<R (Arg)> {
    template <
        typename T, typename E, typename F,
        Utils::enable_if_t<Utils::is_convertible<E, Arg>::value, bool> = true>
    RESULT_MAYBE_UNUSED
    static Result<T, R> map_err(const Result<T, E> &r RESULT_MAYBE_UNUSED, F fun) {
        auto err = r.storage().template get<E>();
        auto ret = fun(std::move(err));

        return Err(std::move(ret));
    }
};

template <typename Arg>
struct MapErr<void (Arg)> {
    template <
        typename T, typename E, typename F,
        Utils::enable_if_t<Utils::is_convertible<E, Arg>::value, bool> = true>
    RESULT_MAYBE_UNUSED
    static Result<T, void> map_err(const Result<T, E> &r RESULT_MAYBE_UNUSED, F fun) {
        auto err = r.storage().template get<E>();
        fun(std::move(err));

        return Wrapper::Err<void>();
    }
};

template <typename R>
struct MapErr<R (void)> {
    template <typename T, typename E, typename F>
    RESULT_MAYBE_UNUSED
    static Result<T, R> map_err(const Result<T, E> &r RESULT_MAYBE_UNUSED, F fun) {
        auto ret = fun();
        return Err(std::move(ret));
    }
};

template <>
struct MapErr<void (void)> {
    template <typename T, typename E, typename F>
    RESULT_MAYBE_UNUSED
    static Result<T, void> map_err(const Result<T, E> &r RESULT_MAYBE_UNUSED, F fun) {
        fun();
        return Wrapper::Err<void>();
    }
};

} // Impl





/**
 * SFINAE resolver namespace
 */
namespace Resolve {

// map
template<typename F>
struct Map : public Impl::Map<decltype(&F::operator())> {};

template<typename R, typename ...Args>
struct Map<R (*)(Args...)> : public Impl::Map<R (Args...)> {};

template<typename R, typename C, typename ...Args>
struct Map<R (C::*)(Args...)> : public Impl::Map<R (Args...)> {};

template<typename R, typename C, typename ...Args>
struct Map<R (C::*)(Args...) const> : public Impl::Map<R (Args...)> {};

template<typename R, typename ...Args>
struct Map<std::function<R (Args...)>> : public Impl::Map<R (Args...)> {};




// map error
template<typename F>
struct MapErr : public Impl::MapErr<decltype(&F::operator())> {};

template<typename R, typename ...Args>
struct MapErr<R (*)(Args...)> : public Impl::MapErr<R (Args...)> {};

template<typename R, typename C, typename ...Args>
struct MapErr<R (C::*)(Args...)> : public Impl::MapErr<R (Args...)> {};

template<typename R, typename C, typename ...Args>
struct MapErr<R (C::*)(Args...) const> : public Impl::MapErr<R (Args...)> {};

template<typename R, typename ...Args>
struct MapErr<std::function<R (Args...)>> : public Impl::MapErr<R (Args...)> {};

} // Resolve




namespace Proxy {
template <
    typename T, typename E, typename F, typename R = typename Utils::result_of<F>::type>
static Result<R, E> map(const Result<T, E> &res, F fun) {
    return Resolve::Map<F>::map(res, fun);
}

template <
    typename T, typename E, typename F, typename R = typename Utils::result_of<F>::type>
static Result<T, R> map_err(const Result<T, E> &res, F fun) {
    return Resolve::MapErr<F>::map_err(res, fun);
}

} // Proxy namespace





template <typename T, typename E>
struct Result {
private:
    Utils::Storage<T, E> _storage;

public:

    /**
     * @brief   Allows the construction of a Result<T, E> instance through rvalue's of type
     *          Ok<T> and Err<E>.
     */
    RESULT_NODISCARD Result(Wrapper::Ok<T> &&s) noexcept : _storage { std::move(s) } {}
    RESULT_NODISCARD Result(Wrapper::Err<E> &&s) noexcept : _storage { std::move(s) } {}





    /**
     * @brief   Allows the construction of a Result<T, E> instance through an rvalue of type
     *          Result<T, E>.
     */
    RESULT_NODISCARD Result(Result<T, E> &&o) noexcept : _storage { std::move(o._storage) } {}
    RESULT_NODISCARD Result<T, E> &operator=(Result<T, E> &&o) noexcept {
        this->_storage = std::move(o._storage);
        return *this;
    }





    /**
     * @brief   Results should not be copyable.
     */
    RESULT_NODISCARD Result(const Result &o) =delete;
    RESULT_NODISCARD Result<T, E> &operator=(const Result &o) noexcept =delete;





    /**
     * @brief   Reference to the underlying container storing either an instance of T or E.
     */
    RESULT_MAYBE_UNUSED decltype(Result<T, E>::_storage) &storage() {
        return this->_storage;
    }

    RESULT_MAYBE_UNUSED const decltype(Result<T, E>::_storage) &storage() const {
        return this->_storage;
    }





    /**
     * @brief   Queries information about whether the contained result is Ok<T> or Err<E>
     */
    RESULT_MAYBE_UNUSED RESULT_NODISCARD bool is_ok() const {
        return storage().template holds_alternative<Wrapper::Ok<T>>();
    }

    RESULT_MAYBE_UNUSED RESULT_NODISCARD bool is_err() const {
        return storage().template holds_alternative<Wrapper::Err<E>>();
    }





    /**
     * @brief   Only available if the type T of Ok<T> is not void.
     *          Moves the contained instance of type T when the Result<T, E> contains Ok<T>.
     *          However in case of containing an Err<E> it will fail.
     */
    template <
        typename _ = typename std::decay<T>::type,
        typename R = typename std::enable_if<!std::is_same<_, void>::value, _>::type>
    RESULT_MAYBE_UNUSED R unwrap() {
        if (is_ok()) {
            return std::move(storage().template get<R>());
        }

        RESULT_ERROR("Tried to unwrap a result containing an error");
    }





    /**
     * @brief   Only available if the type E of Err<E> is not void.
     *          Moves the contained instance of type E when the Result<T, E> contains Err<E>.
     *          However in case of containing an Ok<T> it will fail.
     */
    template <
        typename _ = typename std::decay<E>::type,
        typename R = typename std::enable_if<!std::is_same<_, void>::value, _>::type>
    RESULT_MAYBE_UNUSED R unwrap_err() {
        if (is_err()) {
            return std::move(storage().template get<R>());
        }

        RESULT_ERROR("Tried to unwrap an error containing a result");
    }





    /**
     * @brief   Unchecked variants of unwrap and unwrap_err.
     *          These should only be used if we have absolute certainty abut the desired result.
     *          Moves the contained T (or E) from the result.
     */
    template <
        typename _ = typename std::decay<E>::type,
        typename R = typename std::enable_if<!std::is_same<_, void>::value, _>::type>
    RESULT_MAYBE_UNUSED R unwrap_unchecked() {
        return std::move(storage().template get<R>());
    }

    template <
        typename _ = typename std::decay<E>::type,
        typename R = typename std::enable_if<!std::is_same<_, void>::value, _>::type>
    RESULT_MAYBE_UNUSED R unwrap_err_unchecked() {
        return std::move(storage().template get<R>());
    }





    /**
     * @brief   Only available if the type T of Ok<T> is not void.
     *          Returns a reference to the contained instance of type T when the Result<T, E> is
     *          Ok<T>. However in case of Err<E> it will return a reference to the
     *          explicitly stated reference of type T.
     */
    template <
        typename _ = typename std::decay<T>::type,
        typename R = typename std::enable_if<!std::is_same<_, void>::value, _>::type>
    RESULT_MAYBE_UNUSED R unwrap_or(const _ &t) {
        if (is_ok()) {
            return std::move(storage().template get<T>());
        }

        return t;
    }





    /**
     * @brief   Only available if type T of Ok<T> can be default constructed and if T is not void.
     *          In case the Result<T, E> contains Err<E> this function returns
     *          a default constructed instance of T, else it moves the contained instance of T.
     */
    template <
        typename _ = typename std::decay<T>::type,
        typename R = typename std::enable_if<!std::is_same<_, void>::value, _>::type,
        Utils::enable_if_t<std::is_default_constructible<R>::value, bool> = true>
    RESULT_MAYBE_UNUSED R unwrap_or_default() {
        if (is_err()) {
            return R {};
        }

        return std::move(storage().template get<R>());
    }





    /**
     * @brief   Only available if the type T of Ok<T> is not void.
     *          Moves the contained instance of T when the Result<T, E> contains Ok<T>.
     *          However in case of Err<E> it will fail like unwrap() but display the
     *          specific given string.
     */
    template <
        typename _ = typename std::decay<T>::type,
        typename R = typename std::enable_if<!std::is_same<_, void>::value, _>::type>
    RESULT_MAYBE_UNUSED R expect(const std::string s) {
        if (is_ok()) {
            return std::move(storage().template get<R>());
        }

        RESULT_ERROR(s);
    }





    /**
     * @brief   Transforms a Result<T, E> to Result<R, E>.
     *          If the result contains an Err<E> nothing happens.
     *          The function used for the mapping must either discard T (no arg) or take exactly
     *          one argument which T needs to be implicit convertible to.
     *          Consumes the underlying Result<T, E>.
     */
    template <
        typename F,
        typename R = typename Utils::result_of<F>::type,
        Utils::enable_if_t<!Utils::is_result<R>::value, bool> = true>
    RESULT_MAYBE_UNUSED Result<R, E> map(F fun) {
        if (is_ok()) {
            return Proxy::map(*this, fun);
        }

        // constructs a Result<R, E> from Err<E> contained in Result<T, E>
        return Wrapper::Get<Wrapper::Err<E>>::get(*this);
    }





    /**
     * @brief   Transforms a Result<T, E> to Result<T, R>.
     *          If the result contains an Ok<T> nothing happens.
     *          The function used for the mapping must either discard E (no arg) or take exactly
     *          one argument which E needs to be implicit convertible to.
     *          Consumes the underlying Result<T, E>.
     */
    template <
        typename F,
        typename R = typename Utils::result_of<F>::type,
        Utils::enable_if_t<!Utils::is_result<R>::value, bool> = true>
    RESULT_MAYBE_UNUSED Result<T, R> map_err(F fun) {
        if (is_err()) {
            return Proxy::map_err(*this, fun);
        }

        // constructs a Result<R, E> from Err<E> contained in Result<T, E>
        return Wrapper::Get<Wrapper::Ok<T>>::get(*this);
    }





    /**
     * @brief   Transforms a Result<T, E> to Result<R, T>.
     *          If the result contains an Err<E> the specified instance of type R will be
     *          returned via reference, else the transformation is applied and the result
     *          Consumes the underlying base Result<T, E>.
     */
    template <
        typename F,
        typename R = typename Utils::result_of<F>::type,
        Utils::enable_if_t<!std::is_same<R, void>::value, bool> = true>
    RESULT_MAYBE_UNUSED R map_or(F fun, R r) {
        if (is_ok()) {
            return std::move(Proxy::map(*this, fun).storage().template get<R>());
        }

        return r;
    }





    /**
     * @brief   Transforms a Result<T, E> to R if the result contains an Ok<T>.
     *          Transforms a Result<T, E> to R if the result contains an Err<E>.
     *          These functions do not need to be the same.
     *          Consumes the underlying base Result<T, E>.
     */
    template <
        typename F,
        typename O,
        typename R = typename Utils::result_of<F>::type,
        Utils::enable_if_t<std::is_same<R, typename Utils::result_of<O>::type>::value, bool> = true>
    RESULT_MAYBE_UNUSED R map_or_else(F fun, O other) {
        if (is_ok()) {
            return std::move(Proxy::map(*this, fun).storage().template get<R>());
        }
        else {
            return std::move(Proxy::map_err(*this, other).storage().template get<R>());
        }
    }





    /**
     * @brief   Transforms a Result<T, E> to Result<U, E> is the result contains OK<T> else it
     *          returns Err<E> contained in the result.
     *          Consumes the underlying Result<T, E>.
     */
    template <
        typename F,
        typename R = typename Utils::result_of<F>::type,
        typename U = typename Utils::destruct_result<R>::Ok>
    RESULT_MAYBE_UNUSED Result<U, E> and_then(F fun) {
        if (is_ok()) {
            return std::move(Proxy::map(*this, fun).storage().template get<R>());
        }

        // constructs Result<U, E> from Err<E>
        return Wrapper::Get<Wrapper::Err<E>>::get(*this);
    }





    /**
     * @brief   Transforms a Result<T, E> to Result<U, E> is the result contains OK<T> else it
     *          returns Err<E> contained in the result.
     *          Consumes the underlying Result<T, E>.
     */
    template <
        typename F,
        typename R = typename Utils::result_of<F>::type,
        typename U = typename Utils::destruct_result<R>::Err>
    RESULT_MAYBE_UNUSED Result<T, U> or_else(F fun) {
        if (is_err()) {
            return std::move(Proxy::map_err(*this, fun).storage().template get<R>());
        }

        // constructs Result<U, E> from Err<E>
        return Wrapper::Get<Wrapper::Ok<T>>::get(*this);
    }





    /**
     * @brief   Only available if T is equality comparable.
     *          Returns true if both Result<T, E> and Ok<T> contain Ok<T>
     *          and the instances of T are equal, else false.
     */
    template <
        typename U,
        Utils::enable_if_t<Utils::is_eq_comparable<U, T>::value, bool> = true>
    bool operator==(const Wrapper::Ok<U> &other) {
        return is_ok() && (storage().template get<T>() == other._t);
    }





    /**
     * @brief   Only available if E is equality comparable.
     *          Returns true if both Result<T, E> and Err<E> contain Err<E>
     *          and the instances of E are equal, else false.
     */
    template <
        typename U,
        Utils::enable_if_t<Utils::is_eq_comparable<U, E>::value, bool> = true>
    bool operator==(const Wrapper::Err<U> &other) {
        return is_err() && (storage().template get<E>() == other._e);
    }





    /**
     * @brief   Only available if T but no E is equality comparable.
     *          Returns true if both results contain Ok<T> and the instances of T are equal,
     *          else false.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t< Utils::is_eq_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t<!Utils::is_eq_comparable<G, E>::value, bool> = true>
    bool operator==( const Result<F, G> &other) {
        return 
            (is_ok() && other.is_ok()) && 
            (storage().template get<T>() == other.storage().template get<F>());
    }





    /**
     * @brief   Only available if E but no T is equality comparable.
     *          Returns true if both results contain Err<E> and the instances of E are equal,
     *          else false.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t<!Utils::is_eq_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t< Utils::is_eq_comparable<G, E>::value, bool> = true>
    bool operator==(const Result<F, G> &other) {
        return 
            (is_err() && other.is_err()) && 
            (storage().template get<E>() == other.storage().template get<G>());
    }

    



    /**
     * @brief   Only available if T and E are equality comparable.
     *          Returns true if both results contain Ok<T> and the instances of T are equal.
     *          Returns true if both results contain Err<E> and the instances of E are equal.
     *          Else returns false.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t<Utils::is_eq_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t<Utils::is_eq_comparable<G, E>::value, bool> = true>
    bool operator==(const Result<F, G> &other) {
        if (is_ok() && other.is_ok()) {
            return storage().template get<T>() == other.storage().template get<F>();
        }

        if (is_err() && other.is_err()) {
            return storage().template get<E>() == other.storage().template get<G>();
        }

        return false;
    }
    
    
    
    
    
    /**
     * @brief   Only available if T is not-equal comparable.
     *          Returns true if both Result<T, E> and Ok<T> contain Ok<T>
     *          and the instances of T are equal, else false.
     */
    template <
        typename U,
        Utils::enable_if_t<Utils::is_ne_comparable<U, T>::value, bool> = true>
    bool operator!=(const Wrapper::Ok<U> &other) {
        return is_ok() && (storage().template get<T>() != other._t);
    }





    /**
     * @brief   Only available if E is equality comparable.
     *          Returns true if both Result<T, E> and Err<E> contain Err<E>
     *          and the instances of E are equal, else false.
     */
    template <
        typename U,
        Utils::enable_if_t<Utils::is_ne_comparable<U, E>::value, bool> = true>
    bool operator!=(const Wrapper::Err<U> &other) {
        return is_err() && (storage().template get<E>() != other._e);
    }
    
    
    
    
    
    /**
     * @brief   Only available if T but no E is equality comparable.
     *          Returns false if both results contain Ok<T> and the instances of T are equal,
     *          else true.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t< Utils::is_ne_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t<!Utils::is_ne_comparable<G, E>::value, bool> = true>
    bool operator!=(const Result<F, G> &other) {
        return
            (is_ok() && other.is_ok()) &&
            (storage().template get<T>() != other.storage().template get<F>());
    }





    /**
     * @brief   Only available if E but no T is equality comparable.
     *          Returns false if both results contain Err<E> and the instances of E are equal,
     *          else true.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t<!Utils::is_ne_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t< Utils::is_ne_comparable<G, E>::value, bool> = true>
    bool operator!=(const Result<F, G> &other) {
        return
            (is_err() && other.is_err()) &&
            (storage().template get<E>() != other.storage().template get<G>());
    }

    /**
     * @brief   Only available if T and E are not-equal comparable.
     *          Returns true if this result contains Ok<T>, other contains Ok<F>, 
     *          and the instance of T is not equal to the instance of F.
     *          Returns true if this result contains Err<E>, other contains Err<G>, 
     *          and the instance of E is not equal to instance of G.
     *          Returns false otherwise.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t<Utils::is_ne_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t<Utils::is_ne_comparable<G, E>::value, bool> = true>
    bool operator!=(const Result<F, G> &other) {
        if (is_ok() && other.is_ok()) {
            return storage().template get<T>() != other.storage().template get<F>();
        }

        if (is_err() && other.is_err()) {
            return storage().template get<E>() != other.storage().template get<G>();
        }

        return false;
    }





    /**
     * @brief   Only available if T is less-than comparable.
     *          Returns true if this result contains Ok<T>
     *          and the instance of T is less than the instance in other,
     *          else false.
     */
    template <
        typename U,
        Utils::enable_if_t<Utils::is_lt_comparable<U, T>::value, bool> = true>
    bool operator<(const Wrapper::Ok<U> &other) {
        return is_ok() && (storage().template get<T>() < other._t);
    }





    /**
     * @brief   Only available if E is less-than comparable.
     *          Returns true if this result contains Err<E>
     *          and the instance of E is less than the instance in other,
     *          else false.
     */
    template <
        typename U,
        Utils::enable_if_t<Utils::is_lt_comparable<U, E>::value, bool> = true>
    bool operator<(const Wrapper::Err<U> &other) {
        return is_err() && (storage().template get<E>() < other._e);
    }





    /**
     * @brief   Only available if T but no E is less-than comparable.
     *          Returns true if both results contain Ok<T> and the instance of T
     *          is less than the instance of F, else false.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t< Utils::is_lt_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t<!Utils::is_lt_comparable<G, E>::value, bool> = true>
    bool operator<(const Result<F, G> &other) {
        return 
            (is_ok() && other.is_ok()) && 
            (storage().template get<T>() < other.storage().template get<F>());
    }





    /**
     * @brief   Only available if E but no T is less-than comparable.
     *          Returns true if both results contain Err<E> and the instance of E
     *          is less than the instance of G, else false.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t<!Utils::is_lt_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t< Utils::is_lt_comparable<G, E>::value, bool> = true>
    bool operator<(const Result<F, G> &other) {
        return 
            (is_err() && other.is_err()) && 
            (storage().template get<E>() < other.storage().template get<G>());
    }


    
    
    
    /**
     * @brief   Only available if T and E are less-than comparable.
     *          Returns true if this result contains Ok<T>, other contains Ok<F>, 
     *          and the instance of T is less than the instance of F.
     *          Returns true if this result contains Err<E>, other contains Err<G>, 
     *          and the instance of E is less than the instance of G.
     *          Returns false otherwise.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t<Utils::is_lt_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t<Utils::is_lt_comparable<G, E>::value, bool> = true>
    bool operator<(const Result<F, G> &other) {
        if (is_ok() && other.is_ok()) {
            return storage().template get<T>() < other.storage().template get<F>();
        }

        if (is_err() && other.is_err()) {
            return storage().template get<E>() < other.storage().template get<G>();
        }

        return false;
    }





    /**
     * @brief   Only available if T is less-than-or-equal comparable.
     *          Returns true if this result contains Ok<T>
     *          and the instance of T is less than or equal to the instance in other,
     *          else false.
     */
    template <
        typename U,
        Utils::enable_if_t<Utils::is_le_comparable<U, T>::value, bool> = true>
    bool operator<=(const Wrapper::Ok<U> &other) {
        return is_ok() && (storage().template get<T>() <= other._t);
    }





    /**
     * @brief   Only available if E is less-than-or-equal comparable.
     *          Returns true if this result contains Err<E>
     *          and the instance of E is less than or equal to the instance in other,
     *          else false.
     */
    template <
        typename U,
        Utils::enable_if_t<Utils::is_le_comparable<U, E>::value, bool> = true>
    bool operator<=(const Wrapper::Err<U> &other) {
        return is_err() && (storage().template get<E>() <= other._e);
    }





    /**
     * @brief   Only available if T but no E is less-than-or-equal comparable.
     *          Returns true if both results contain Ok<T> and the instance of T
     *          is less than or equal to the instance of F, else false.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t< Utils::is_le_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t<!Utils::is_le_comparable<G, E>::value, bool> = true>
    bool operator<=(const Result<F, G> &other) {
        return 
            (is_ok() && other.is_ok()) && 
            (storage().template get<T>() <= other.storage().template get<F>());
            
    }





    /**
     * @brief   Only available if E but no T is less-than-or-equal comparable.
     *          Returns true if both results contain Err<E> and the instance of E
     *          is less than or equal to the instance of G, else false.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t<!Utils::is_le_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t< Utils::is_le_comparable<G, E>::value, bool> = true>
    bool operator<=(const Result<F, G> &other) {
        return 
            (is_err() && other.is_err()) && 
            (storage().template get<E>() <= other.storage().template get<G>());
    }





    /**
     * @brief   Only available if T and E are less-than-or-equal comparable.
     *          Returns true if this result contains Ok<T>, other contains Ok<F>,
     *          and the instance of T is less than or equal to the instance of F.
     *          Returns true if this result contains Err<E>, other contains Err<G>,
     *          and the instance of E is less than or equal to the instance of G.
     *          Returns false otherwise.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t<Utils::is_le_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t<Utils::is_le_comparable<G, E>::value, bool> = true>
    bool operator<=(const Result<F, G> &other) {
        if (is_ok() && other.is_ok()) {
            return storage().template get<T>() <= other.storage().template get<F>();
        }

        if (is_err() && other.is_err()) {
            return storage().template get<E>() <= other.storage().template get<G>();
        }

        return false;
    }





    /**
     * @brief   Only available if T is greater-than comparable.
     *          Returns true if this result contains Ok<T>
     *          and the instance of T is greater than the instance in other,
     *          else false.
     */
    template <
        typename U,
        Utils::enable_if_t<Utils::is_gt_comparable<U, T>::value, bool> = true>
    bool operator>(const Wrapper::Ok<U> &other) {
        return is_ok() && (storage().template get<T>() > other._t);
    }





    /**
     * @brief   Only available if E is greater-than comparable.
     *          Returns true if this result contains Err<E>
     *          and the instance of E is greater than the instance in other,
     *          else false.
     */
    template <
        typename U,
        Utils::enable_if_t<Utils::is_gt_comparable<U, E>::value, bool> = true>
    bool operator>(const Wrapper::Err<U> &other) {
        return is_err() && (storage().template get<E>() > other._e);
    }





    /**
     * @brief   Only available if T but no E is greater-than comparable.
     *          Returns true if both results contain Ok<T> and the instance of T
     *          is greater than the instance of F, else false.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t< Utils::is_gt_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t<!Utils::is_gt_comparable<G, E>::value, bool> = true>
    bool operator>(const Result<F, G> &other) {
        return 
            (is_ok() && other.is_ok()) && 
            (storage().template get<T>() > other.storage().template get<F>());
    }





    /**
     * @brief   Only available if E but no T is greater-than comparable.
     *          Returns true if both results contain Err<E> and the instance of E
     *          is greater than the instance of G, else false.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t<!Utils::is_gt_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t< Utils::is_gt_comparable<G, E>::value, bool> = true>
    bool operator>(const Result<F, G> &other) {
        return 
            (is_err() && other.is_err()) &&
            (storage().template get<E>() > other.storage().template get<G>());
    }





    /**
     * @brief   Only available if T and E are greater-than comparable.
     *          Returns true if this result contains Ok<T>, other contains Ok<F>,
     *          and the instance of T is greater than the instance of F.
     *          Returns true if this result contains Err<E>, other contains Err<G>,
     *          and the instance of E is greater than the instance of G.
     *          Returns false otherwise.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t<Utils::is_gt_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t<Utils::is_gt_comparable<G, E>::value, bool> = true>
    bool operator>(const Result<F, G> &other) {
        if (is_ok() && other.is_ok()) {
            return storage().template get<T>() > other.storage().template get<F>();
        }

        if (is_err() && other.is_err()) {
            return storage().template get<E>() > other.storage().template get<G>();
        }

        return false;
    }





    /**
     * @brief   Only available if T is greater-than-or-equal comparable.
     *          Returns true if this result contains Ok<T>
     *          and the instance of T is greater than or equal to the instance in other,
     *          else false.
     */
    template <
        typename U,
        Utils::enable_if_t<Utils::is_ge_comparable<U, T>::value, bool> = true>
    bool operator>=(const Wrapper::Ok<U> &other) {
        return is_ok() && (storage().template get<T>() >= other._t);
    }





    /**
     * @brief   Only available if E is greater-than-or-equal comparable.
     *          Returns true if this result contains Err<E>
     *          and the instance of E is greater than or equal to the instance in other,
     *          else false.
     */
    template <
        typename U,
        Utils::enable_if_t<Utils::is_ge_comparable<U, E>::value, bool> = true>
    bool operator>=(const Wrapper::Err<U> &other) {
        return is_err() && (storage().template get<E>() >= other._e);
    }





    /**
     * @brief   Only available if T but no E is greater-than-or-equal comparable.
     *          Returns true if both results contain Ok<T> and the instance of T
     *          is greater than or equal to the instance of F, else false.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t< Utils::is_ge_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t<!Utils::is_ge_comparable<G, E>::value, bool> = true>
    bool operator>=(const Result<F, G> &other) {
        return 
            (is_ok() && other.is_ok()) && 
            (storage().template get<T>() >= other.storage().template get<F>());
    }





    /**
     * @brief   Only available if E but no T is greater-than-or-equal comparable.
     *          Returns true if both results contain Err<E> and the instance of E
     *          is greater than or equal to the instance of G, else false.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t<!Utils::is_ge_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t< Utils::is_ge_comparable<G, E>::value, bool> = true>
    bool operator>=(const Result<F, G> &other) {
        return
            (is_err() && other.is_err()) &&
            (storage().template get<E>() >= other.storage().template get<G>());
    }





    /**
     * @brief   Only available if T and E are greater-than-or-equal comparable.
     *          Returns true if this result contains Ok<T>, other contains Ok<F>,
     *          and the instance of T is greater than or equal to the instance of F.
     *          Returns true if this result contains Err<E>, other contains Err<G>,
     *          and the instance of E is greater than or equal to the instance of G.
     *          Returns false otherwise.
     */
    template <
        typename F,
        typename G,
        Utils::enable_if_t<Utils::is_ge_comparable<F, T>::value, bool> = true,
        Utils::enable_if_t<Utils::is_ge_comparable<G, E>::value, bool> = true>
    bool operator>=(const Result<F, G> &other) {
        if (is_ok() && other.is_ok()) {
            return storage().template get<T>() >= other.storage().template get<F>();
        }

        if (is_err() && other.is_err()) {
            return storage().template get<E>() >= other.storage().template get<G>();
        }

        return false;
    }
};


#ifdef RESULT_NAMESPACE
}
#endif

#undef RESULT_MAYBE_UNUSED
#undef RESULT_NODISCARD
#undef RESULT_ERROR

#endif // CPP_RESULT_RESULT_HPP
