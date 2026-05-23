#include <cassert>
#include <cstdint>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../include/result/result.hpp"

// If you compile your Result with RESULT_NAMESPACE defined, uncomment this.
// using namespace lsr::result;

// ================================================================================================
// Compile-time helpers
// ================================================================================================

struct IntToLong {
    long operator()(int x) const { return static_cast<long>(x + 1); }
};

struct IntToVoid {
    void operator()(int) const {}
};

struct VoidToInt {
    int operator()() const { return 123; }
};

struct VoidToVoid {
    void operator()() const {}
};

struct StringToInt {
    int operator()(std::string s) const { return static_cast<int>(s.size()); }
};

struct StringToVoid {
    void operator()(std::string) const {}
};

struct IntToResultIntString {
    Result<int, std::string> operator()(int x) const {
        return Result<int, std::string>(Ok(x + 10));
    }
};

struct StringToResultIntString {
    Result<int, std::string> operator()(std::string e) const {
        return Result<int, std::string>(Err(std::move(e)));
    }
};

struct VoidToResultIntString {
    Result<int, std::string> operator()() const { return Result<int, std::string>(Ok(55)); }
};

struct StringToResultVoidInt {
    Result<void, int> operator()(std::string s) const {
        return Result<void, int>(Err(static_cast<int>(s.size())));
    }
};

// ================================================================================================
// Enum test types
// ================================================================================================

enum class SmallEnum : std::uint8_t { a = 0, b = 1, c = 2 };

enum class SparseSmallEnum : std::uint8_t { x = 10, y = 20, z = 30 };

enum class BigEnum : std::uint32_t { ok = 0u, warning = 1u, error = 2u, max_named = 0xffffffffu };

enum class SignedBigEnum : std::int32_t { negative = -100, zero = 0, positive = 100 };

// ================================================================================================
// Compile-time tests
// ================================================================================================

static_assert(std::is_same_v<Result<int, std::string>::ok_type, int>);
static_assert(std::is_same_v<Result<int, std::string>::err_type, std::string>);

static_assert(std::is_same_v<Result<void, std::string>::ok_type, void>);
static_assert(std::is_same_v<Result<void, std::string>::err_type, std::string>);

static_assert(std::is_same_v<Result<int, void>::ok_type, int>);
static_assert(std::is_same_v<Result<int, void>::err_type, void>);

static_assert(std::is_same_v<Result<void, void>::ok_type, void>);
static_assert(std::is_same_v<Result<void, void>::err_type, void>);

static_assert(!std::is_default_constructible_v<Result<int, std::string>>);
static_assert(std::is_constructible_v<Result<int, std::string>, wrapper::Ok<int>>);
static_assert(std::is_constructible_v<Result<int, std::string>, wrapper::Err<std::string>>);

static_assert(!std::is_constructible_v<Result<int, std::string>, wrapper::Ok<std::string>>);
static_assert(!std::is_constructible_v<Result<int, std::string>, wrapper::Err<int>>);

static_assert(std::is_same_v<decltype(Ok(1)), wrapper::Ok<int>>);
static_assert(std::is_same_v<decltype(Err(std::string{"x"})), wrapper::Err<std::string>>);

static_assert(std::is_same_v<decltype(Ok()), wrapper::Ok<void>>);
static_assert(std::is_same_v<decltype(Err()), wrapper::Err<void>>);

static_assert(
    std::is_same_v<decltype(std::declval<Result<int, std::string>&>().unwrap_ref()), int&>);

static_assert(std::is_same_v<decltype(std::declval<const Result<int, std::string>&>().unwrap_ref()),
                             const int&>);

static_assert(std::is_same_v<decltype(std::declval<Result<int, std::string>&>().unwrap_err_ref()),
                             std::string&>);

static_assert(
    std::is_same_v<decltype(std::declval<const Result<int, std::string>&>().unwrap_err_ref()),
                   const std::string&>);

static_assert(
    std::is_same_v<decltype(std::declval<Result<int&, std::string>&>().unwrap_ref()), int&>);

static_assert(
    std::is_same_v<decltype(std::declval<const Result<int&, std::string>&>().unwrap_ref()), int&>);

static_assert(std::is_same_v<decltype(std::declval<Result<int, std::string&>&>().unwrap_err_ref()),
                             std::string&>);

static_assert(
    std::is_same_v<decltype(std::declval<const Result<int, std::string&>&>().unwrap_err_ref()),
                   std::string&>);

static_assert(std::is_same_v<decltype(std::declval<Result<int, std::string>>().unwrap()), int>);

static_assert(std::is_same_v<decltype(std::declval<Result<int&, std::string>>().unwrap()), int&>);

static_assert(
    std::is_same_v<decltype(std::declval<Result<int, std::string>>().unwrap_err()), std::string>);

static_assert(
    std::is_same_v<decltype(std::declval<Result<int, std::string&>>().unwrap_err()), std::string&>);

static_assert(std::is_same_v<decltype(std::declval<Result<int, std::string>>().map(IntToLong{})),
                             Result<long, std::string>>);

static_assert(std::is_same_v<decltype(std::declval<Result<int, std::string>>().map(IntToVoid{})),
                             Result<void, std::string>>);

using MapErrRet1 = decltype(std::declval<Result<int, std::string>>().map_err(StringToInt{}));
static_assert(std::is_same_v<typename MapErrRet1::ok_type, int>);
static_assert(std::is_same_v<typename MapErrRet1::err_type, int>);

using VoidMapRet1 = decltype(std::declval<Result<void, std::string>>().map(VoidToVoid{}));
static_assert(std::is_same_v<typename VoidMapRet1::ok_type, void>);
static_assert(std::is_same_v<typename VoidMapRet1::err_type, std::string>);

static_assert(std::is_same_v<decltype(std::declval<Result<void, std::string>>().map(VoidToInt{})),
                             Result<int, std::string>>);

static_assert(std::is_same_v<decltype(std::declval<Result<void, std::string>>().map(VoidToVoid{})),
                             Result<void, std::string>>);

static_assert(std::is_same_v<decltype(std::declval<Result<int, void>>().map(IntToLong{})),
                             Result<long, void>>);

static_assert(std::is_same_v<decltype(std::declval<Result<int, void>>().map(IntToVoid{})),
                             Result<void, void>>);

static_assert(std::is_same_v<decltype(std::declval<Result<void, void>>().map(VoidToInt{})),
                             Result<int, void>>);

static_assert(std::is_same_v<decltype(std::declval<Result<void, void>>().map(VoidToVoid{})),
                             Result<void, void>>);

static_assert(std::is_same_v<
              decltype(std::declval<Result<int, std::string>>().and_then(IntToResultIntString{})),
              Result<int, std::string>>);

static_assert(std::is_same_v<
              decltype(std::declval<Result<int, std::string>>().or_else(StringToResultIntString{})),
              Result<int, std::string>>);

static_assert(std::is_same_v<
              decltype(std::declval<Result<void, std::string>>().and_then(VoidToResultIntString{})),
              Result<int, std::string>>);

static_assert(std::is_same_v<
              decltype(std::declval<Result<void, std::string>>().or_else(StringToResultVoidInt{})),
              Result<void, int>>);

// ================================================================================================
// Compile-time enum niche tests
// ================================================================================================

static_assert(std::is_same_v<Result<SmallEnum, void>::ok_type, SmallEnum>);
static_assert(std::is_same_v<Result<SmallEnum, void>::err_type, void>);

static_assert(std::is_same_v<Result<void, SmallEnum>::ok_type, void>);
static_assert(std::is_same_v<Result<void, SmallEnum>::err_type, SmallEnum>);

static_assert(std::is_same_v<Result<BigEnum, void>::ok_type, BigEnum>);
static_assert(std::is_same_v<Result<BigEnum, void>::err_type, void>);

static_assert(std::is_same_v<Result<void, BigEnum>::ok_type, void>);
static_assert(std::is_same_v<Result<void, BigEnum>::err_type, BigEnum>);

static_assert(sizeof(Result<SmallEnum, void>) == sizeof(SmallEnum));
static_assert(sizeof(Result<SparseSmallEnum, void>) == sizeof(SparseSmallEnum));
static_assert(sizeof(Result<BigEnum, void>) == sizeof(BigEnum));
static_assert(sizeof(Result<SignedBigEnum, void>) == sizeof(SignedBigEnum));

static_assert(sizeof(Result<void, SmallEnum>) == sizeof(SmallEnum));
static_assert(sizeof(Result<void, SparseSmallEnum>) == sizeof(SparseSmallEnum));
static_assert(sizeof(Result<void, BigEnum>) == sizeof(BigEnum));
static_assert(sizeof(Result<void, SignedBigEnum>) == sizeof(SignedBigEnum));

static_assert(std::is_constructible_v<Result<SmallEnum, void>, wrapper::Ok<SmallEnum>>);
static_assert(std::is_constructible_v<Result<SmallEnum, void>, wrapper::Err<void>>);

static_assert(std::is_constructible_v<Result<void, SmallEnum>, wrapper::Ok<void>>);
static_assert(std::is_constructible_v<Result<void, SmallEnum>, wrapper::Err<SmallEnum>>);

static_assert(std::is_constructible_v<Result<BigEnum, void>, wrapper::Ok<BigEnum>>);
static_assert(std::is_constructible_v<Result<BigEnum, void>, wrapper::Err<void>>);

static_assert(std::is_constructible_v<Result<void, BigEnum>, wrapper::Ok<void>>);
static_assert(std::is_constructible_v<Result<void, BigEnum>, wrapper::Err<BigEnum>>);

static_assert(
    std::is_same_v<decltype(std::declval<Result<SmallEnum, void>&>().unwrap_ref()), SmallEnum&>);

static_assert(std::is_same_v<decltype(std::declval<const Result<SmallEnum, void>&>().unwrap_ref()),
                             const SmallEnum&>);

static_assert(std::is_same_v<decltype(std::declval<Result<void, SmallEnum>&>().unwrap_err_ref()),
                             SmallEnum&>);

static_assert(
    std::is_same_v<decltype(std::declval<const Result<void, SmallEnum>&>().unwrap_err_ref()),
                   const SmallEnum&>);

static_assert(
    std::is_same_v<decltype(std::declval<Result<BigEnum, void>&>().unwrap_ref()), BigEnum&>);

static_assert(std::is_same_v<decltype(std::declval<const Result<BigEnum, void>&>().unwrap_ref()),
                             const BigEnum&>);

static_assert(
    std::is_same_v<decltype(std::declval<Result<void, BigEnum>&>().unwrap_err_ref()), BigEnum&>);

static_assert(
    std::is_same_v<decltype(std::declval<const Result<void, BigEnum>&>().unwrap_err_ref()),
                   const BigEnum&>);

static_assert(tiny::impl::automatic_enum_sentinel<SmallEnum> == static_cast<SmallEnum>(3));
static_assert(tiny::impl::automatic_enum_sentinel<SparseSmallEnum> ==
              static_cast<SparseSmallEnum>(0));

static_assert(tiny::impl::automatic_enum_sentinel<BigEnum> != BigEnum::ok);
static_assert(tiny::impl::automatic_enum_sentinel<BigEnum> != BigEnum::warning);
static_assert(tiny::impl::automatic_enum_sentinel<BigEnum> != BigEnum::error);
static_assert(tiny::impl::automatic_enum_sentinel<BigEnum> != BigEnum::max_named);

static_assert(tiny::impl::automatic_enum_sentinel<SignedBigEnum> != SignedBigEnum::negative);
static_assert(tiny::impl::automatic_enum_sentinel<SignedBigEnum> != SignedBigEnum::zero);
static_assert(tiny::impl::automatic_enum_sentinel<SignedBigEnum> != SignedBigEnum::positive);

// ================================================================================================
// Runtime tests
// ================================================================================================

static void test_ok_err_wrapper_basics() {
    int  x = 10;
    auto ok_ref = Ok(x);
    auto err_ref = Err(x);

    static_assert(std::is_same_v<decltype(ok_ref), wrapper::Ok<int&>>);
    static_assert(std::is_same_v<decltype(err_ref), wrapper::Err<int&>>);

    assert(ok_ref.value == &x);
    assert(err_ref.value == &x);

    auto ok_value = Ok(20);
    auto err_value = Err(30);

    static_assert(std::is_same_v<decltype(ok_value), wrapper::Ok<int>>);
    static_assert(std::is_same_v<decltype(err_value), wrapper::Err<int>>);

    assert(ok_value.value == 20);
    assert(err_value.value == 30);
}

static void test_result_void_void() {
    Result<void, void> ok(Ok());
    Result<void, void> err(Err());

    assert(ok.is_ok());
    assert(!ok.is_err());

    assert(err.is_err());
    assert(!err.is_ok());

    ok.unwrap();
    err.unwrap_err();

    assert(ok == Ok());
    assert(err == Err());
    assert(ok != err);

    auto mapped_ok = std::move(ok).map([] { return 42; });

    static_assert(std::is_same_v<decltype(mapped_ok), Result<int, void>>);
    assert(mapped_ok.is_ok());
    assert(std::move(mapped_ok).unwrap() == 42);

    Result<void, void> err2(Err());

    auto mapped_err = std::move(err2).map([] { return 42; });

    assert(mapped_err.is_err());

    Result<void, void> ok2(Ok());

    auto and_then_ok = std::move(ok2).and_then([] { return Result<int, void>(Ok(7)); });

    assert(and_then_ok.is_ok());
    assert(std::move(and_then_ok).unwrap() == 7);

    Result<void, void> err3(Err());

    auto or_else_err = std::move(err3).or_else([] { return Result<void, int>(Err(99)); });

    assert(or_else_err.is_err());
    assert(std::move(or_else_err).unwrap_err() == 99);
}

static void test_result_t_void_ok_path() {
    Result<int, void> r(Ok(42));

    assert(r.is_ok());
    assert(!r.is_err());
    assert(r.unwrap_ref() == 42);
    assert(std::move(r).unwrap() == 42);

    Result<int, void> r2(Ok(11));

    auto mapped = std::move(r2).map([](int x) { return x * 2; });

    static_assert(std::is_same_v<decltype(mapped), Result<int, void>>);
    assert(mapped.is_ok());
    assert(std::move(mapped).unwrap() == 22);

    Result<int, void> r3(Ok(5));

    auto mapped_void = std::move(r3).map([](int x) { assert(x == 5); });

    static_assert(std::is_same_v<decltype(mapped_void), Result<void, void>>);
    assert(mapped_void.is_ok());

    Result<int, void> r4(Ok(8));

    auto chained = std::move(r4).and_then(
        [](int x) { return Result<std::string, void>(Ok(std::to_string(x))); });

    static_assert(std::is_same_v<decltype(chained), Result<std::string, void>>);
    assert(chained.is_ok());
    assert(std::move(chained).unwrap() == "8");

    Result<int, void> r5(Ok(9));

    auto or_else_result = std::move(r5).or_else(
        [] { return Result<int, std::string>(Err(std::string{"should not run"})); });

    assert(or_else_result.is_ok());
    assert(std::move(or_else_result).unwrap() == 9);
}

static void test_result_t_void_err_path() {
    Result<int, void> r(Err());

    assert(r.is_err());
    assert(!r.is_ok());
    r.unwrap_err();

    auto mapped = std::move(r).map([](int x) { return x * 2; });

    assert(mapped.is_err());

    Result<int, void> r2(Err());

    auto fallback = std::move(r2).map_or([](int x) { return x * 2; }, 123);

    assert(fallback == 123);

    Result<int, void> r3(Err());

    auto fallback_else = std::move(r3).map_or_else([](int x) { return x * 2; }, [] { return 456; });

    assert(fallback_else == 456);

    Result<int, void> r4(Err());

    auto recovered = std::move(r4).or_else(
        [] { return Result<int, std::string>(Err(std::string{"converted error"})); });

    assert(recovered.is_err());
    assert(std::move(recovered).unwrap_err() == "converted error");

    Result<int, void> r5(Err());
    assert(std::move(r5).unwrap_or_default() == 0);
}

static void test_result_void_e_ok_path() {
    Result<void, std::string> r(Ok());

    assert(r.is_ok());
    assert(!r.is_err());

    r.unwrap();

    auto mapped = std::move(r).map([] { return 77; });

    static_assert(std::is_same_v<decltype(mapped), Result<int, std::string>>);
    assert(mapped.is_ok());
    assert(std::move(mapped).unwrap() == 77);

    Result<void, std::string> r2(Ok());

    auto chained = std::move(r2).and_then([] { return Result<int, std::string>(Ok(88)); });

    assert(chained.is_ok());
    assert(std::move(chained).unwrap() == 88);

    Result<void, std::string> r3(Ok());

    auto unchanged = std::move(r3).or_else(
        [](std::string e) { return Result<void, int>(Err(static_cast<int>(e.size()))); });

    assert(unchanged.is_ok());
}

static void test_result_void_e_err_path() {
    Result<void, std::string> r(Err(std::string{"bad"}));

    assert(r.is_err());
    assert(!r.is_ok());
    assert(r.unwrap_err_ref() == "bad");
    assert(std::move(r).unwrap_err() == "bad");

    Result<void, std::string> r2(Err(std::string{"hello"}));

    auto mapped_err = std::move(r2).map_err([](std::string e) { return e.size(); });

    static_assert(std::is_same_v<decltype(mapped_err), Result<void, std::size_t>>);
    assert(mapped_err.is_err());
    assert(std::move(mapped_err).unwrap_err() == 5);

    Result<void, std::string> r3(Err(std::string{"abc"}));

    auto fallback = std::move(r3).map_or([] { return 1; }, 999);

    assert(fallback == 999);

    Result<void, std::string> r4(Err(std::string{"abcd"}));

    auto fallback_else = std::move(r4).map_or_else(
        [] { return 1; }, [](std::string e) { return static_cast<int>(e.size()); });

    assert(fallback_else == 4);

    Result<void, std::string> r5(Err(std::string{"recover"}));

    auto recovered = std::move(r5).or_else([](std::string e) {
        assert(e == "recover");
        return Result<void, int>(Ok());
    });

    assert(recovered.is_ok());
}

static void test_result_t_e_ok_path() {
    Result<int, std::string> r(Ok(10));

    assert(r.is_ok());
    assert(!r.is_err());
    assert(r.unwrap_ref() == 10);
    assert(std::move(r).unwrap() == 10);

    Result<int, std::string> r2(Ok(21));

    auto mapped = std::move(r2).map([](int x) { return std::to_string(x); });

    static_assert(std::is_same_v<decltype(mapped), Result<std::string, std::string>>);
    assert(mapped.is_ok());
    assert(std::move(mapped).unwrap() == "21");

    Result<int, std::string> r3(Ok(5));

    auto mapped_void = std::move(r3).map([](int x) { assert(x == 5); });

    static_assert(std::is_same_v<decltype(mapped_void), Result<void, std::string>>);
    assert(mapped_void.is_ok());

    Result<int, std::string> r4(Ok(15));

    auto mapped_err_not_called =
        std::move(r4).map_err([](std::string e) { return static_cast<int>(e.size()); });

    static_assert(std::is_same_v<decltype(mapped_err_not_called), Result<int, int>>);
    assert(mapped_err_not_called.is_ok());
    assert(std::move(mapped_err_not_called).unwrap() == 15);

    Result<int, std::string> r5(Ok(6));

    auto chained = std::move(r5).and_then(
        [](int x) { return Result<std::string, std::string>(Ok(std::string(x, 'x'))); });

    assert(chained.is_ok());
    assert(std::move(chained).unwrap() == "xxxxxx");

    Result<int, std::string> r6(Ok(44));

    auto or_else_not_called = std::move(r6).or_else(
        [](std::string e) { return Result<int, int>(Err(static_cast<int>(e.size()))); });

    assert(or_else_not_called.is_ok());
    assert(std::move(or_else_not_called).unwrap() == 44);
}

static void test_result_t_e_err_path() {
    Result<int, std::string> r(Err(std::string{"error"}));

    assert(r.is_err());
    assert(!r.is_ok());
    assert(r.unwrap_err_ref() == "error");
    assert(std::move(r).unwrap_err() == "error");

    Result<int, std::string> r2(Err(std::string{"abc"}));

    auto mapped = std::move(r2).map([](int x) { return x * 2; });

    static_assert(std::is_same_v<decltype(mapped), Result<int, std::string>>);
    assert(mapped.is_err());
    assert(std::move(mapped).unwrap_err() == "abc");

    Result<int, std::string> r3(Err(std::string{"abcd"}));

    auto mapped_err =
        std::move(r3).map_err([](std::string e) { return static_cast<int>(e.size()); });

    static_assert(std::is_same_v<decltype(mapped_err), Result<int, int>>);
    assert(mapped_err.is_err());
    assert(std::move(mapped_err).unwrap_err() == 4);

    Result<int, std::string> r4(Err(std::string{"fail"}));

    auto fallback = std::move(r4).map_or([](int x) { return x * 2; }, 123);

    assert(fallback == 123);

    Result<int, std::string> r5(Err(std::string{"hello"}));

    auto fallback_else = std::move(r5).map_or_else(
        [](int x) { return x * 2; }, [](std::string e) { return static_cast<int>(e.size()); });

    assert(fallback_else == 5);

    Result<int, std::string> r6(Err(std::string{"chain error"}));

    auto chained = std::move(r6).and_then(
        [](int x) { return Result<std::string, std::string>(Ok(std::to_string(x))); });

    assert(chained.is_err());
    assert(std::move(chained).unwrap_err() == "chain error");

    Result<int, std::string> r7(Err(std::string{"recover"}));

    auto recovered = std::move(r7).or_else([](std::string e) {
        assert(e == "recover");
        return Result<int, int>(Ok(2024));
    });

    assert(recovered.is_ok());
    assert(std::move(recovered).unwrap() == 2024);

    Result<int, std::string> r8(Err(std::string{"fallback"}));
    assert(std::move(r8).unwrap_or(111) == 111);

    Result<int, std::string> r9(Err(std::string{"fallback"}));
    assert(std::move(r9).unwrap_or_default() == 0);
}

static void test_reference_ok_type() {
    int x = 10;

    Result<int&, std::string> r(Ok(x));

    assert(r.is_ok());
    assert(&r.unwrap_ref() == &x);
    assert(r.unwrap_ref() == 10);

    r.unwrap_ref() = 25;
    assert(x == 25);

    int& unwrapped = std::move(r).unwrap();
    assert(&unwrapped == &x);
    unwrapped = 30;
    assert(x == 30);

    int                       y = 40;
    Result<int&, std::string> r2(Ok(y));

    auto mapped = std::move(r2).map([](int& ref) {
        ref += 2;
        return ref;
    });

    assert(mapped.is_ok());
    assert(y == 42);
    assert(std::move(mapped).unwrap() == 42);
}

static void test_reference_err_type() {
    std::string e = "original";

    Result<int, std::string&> r(Err(e));

    assert(r.is_err());
    assert(&r.unwrap_err_ref() == &e);
    assert(r.unwrap_err_ref() == "original");

    r.unwrap_err_ref() = "changed";
    assert(e == "changed");

    std::string& unwrapped = std::move(r).unwrap_err();
    assert(&unwrapped == &e);
    unwrapped = "again";
    assert(e == "again");

    std::string               e2 = "abc";
    Result<int, std::string&> r2(Err(e2));

    auto mapped_err = std::move(r2).map_err([](std::string& ref) {
        ref += "def";
        return ref.size();
    });

    assert(mapped_err.is_err());
    assert(e2 == "abcdef");
    assert(std::move(mapped_err).unwrap_err() == 6);
}

static void test_move_only_ok_type() {
    Result<std::unique_ptr<int>, std::string> r(Ok(std::make_unique<int>(123)));

    assert(r.is_ok());
    assert(*r.unwrap_ref() == 123);

    auto ptr = std::move(r).unwrap();
    assert(ptr);
    assert(*ptr == 123);

    Result<std::unique_ptr<int>, std::string> r2(Ok(std::make_unique<int>(10)));

    auto mapped = std::move(r2).map([](std::unique_ptr<int> p) { return *p + 5; });

    assert(mapped.is_ok());
    assert(std::move(mapped).unwrap() == 15);
}

static void test_move_only_err_type() {
    Result<int, std::unique_ptr<int>> r(Err(std::make_unique<int>(321)));

    assert(r.is_err());
    assert(*r.unwrap_err_ref() == 321);

    auto ptr = std::move(r).unwrap_err();
    assert(ptr);
    assert(*ptr == 321);

    Result<int, std::unique_ptr<int>> r2(Err(std::make_unique<int>(20)));

    auto mapped_err = std::move(r2).map_err([](std::unique_ptr<int> p) { return *p + 1; });

    assert(mapped_err.is_err());
    assert(std::move(mapped_err).unwrap_err() == 21);
}

static void test_equality_positive_cases_only() {
    Result<int, std::string> ok1(Ok(1));
    Result<int, std::string> ok2(Ok(1));
    Result<int, std::string> err1(Err(std::string{"x"}));
    Result<int, std::string> err2(Err(std::string{"x"}));

    assert(ok1 == Ok(1));
    assert(err1 == Err(std::string{"x"}));

    assert(ok1 == ok2);
    assert(err1 == err2);

    Result<void, void> vv_ok1(Ok());
    Result<void, void> vv_ok2(Ok());
    Result<void, void> vv_err1(Err());
    Result<void, void> vv_err2(Err());

    assert(vv_ok1 == vv_ok2);
    assert(vv_err1 == vv_err2);
    assert(vv_ok1 != vv_err1);
}

static void test_sentinel_niche_result_t_void() {
    Result<int, void, -1> ok(Ok(5));
    Result<int, void, -1> err(Err());

    assert(ok.is_ok());
    assert(!ok.is_err());
    assert(ok.unwrap_ref() == 5);

    assert(err.is_err());
    assert(!err.is_ok());

    auto mapped = std::move(ok).map([](int x) { return x + 1; });

    assert(mapped.is_ok());
    assert(std::move(mapped).unwrap() == 6);
}

static void test_sentinel_niche_result_void_e() {
    Result<void, int, tiny::UseDefaultValue, -1> ok(Ok());
    Result<void, int, tiny::UseDefaultValue, -1> err(Err(5));

    assert(ok.is_ok());
    assert(!ok.is_err());

    assert(err.is_err());
    assert(!err.is_ok());
    assert(err.unwrap_err_ref() == 5);

    auto mapped_err = std::move(err).map_err([](int x) { return x + 10; });

    assert(mapped_err.is_err());
    assert(std::move(mapped_err).unwrap_err() == 15);
}

static void test_enum_ok_void_err_small_uint8() {
    static_assert(sizeof(Result<SmallEnum, void>) == sizeof(SmallEnum));

    Result<SmallEnum, void> ok(Ok(SmallEnum::b));
    Result<SmallEnum, void> err(Err());

    assert(ok.is_ok());
    assert(!ok.is_err());
    assert(ok.unwrap_ref() == SmallEnum::b);

    assert(err.is_err());
    assert(!err.is_ok());

    auto mapped = std::move(ok).map([](SmallEnum value) {
        assert(value == SmallEnum::b);
        return SmallEnum::c;
    });

    static_assert(std::is_same_v<decltype(mapped), Result<SmallEnum, void>>);
    assert(mapped.is_ok());
    assert(std::move(mapped).unwrap() == SmallEnum::c);

    Result<SmallEnum, void> err2(Err());

    auto fallback =
        std::move(err2).map_or([](SmallEnum value) { return value == SmallEnum::a ? 1 : 2; }, 99);

    assert(fallback == 99);
}

static void test_void_ok_enum_err_small_uint8() {
    static_assert(sizeof(Result<void, SmallEnum>) == sizeof(SmallEnum));

    Result<void, SmallEnum> ok(Ok());
    Result<void, SmallEnum> err(Err(SmallEnum::c));

    assert(ok.is_ok());
    assert(!ok.is_err());

    assert(err.is_err());
    assert(!err.is_ok());
    assert(err.unwrap_err_ref() == SmallEnum::c);

    auto mapped_err = std::move(err).map_err([](SmallEnum value) {
        assert(value == SmallEnum::c);
        return SmallEnum::a;
    });

    static_assert(std::is_same_v<decltype(mapped_err), Result<void, SmallEnum>>);
    assert(mapped_err.is_err());
    assert(std::move(mapped_err).unwrap_err() == SmallEnum::a);

    Result<void, SmallEnum> ok2(Ok());

    auto fallback = std::move(ok2).map_or_else(
        [] { return 123; }, [](SmallEnum value) { return value == SmallEnum::a ? 1 : 2; });

    assert(fallback == 123);
}

static void test_enum_sparse_small_uint8() {
    static_assert(sizeof(Result<SparseSmallEnum, void>) == sizeof(SparseSmallEnum));
    static_assert(sizeof(Result<void, SparseSmallEnum>) == sizeof(SparseSmallEnum));

    Result<SparseSmallEnum, void> ok(Ok(SparseSmallEnum::y));
    Result<SparseSmallEnum, void> err(Err());

    assert(ok.is_ok());
    assert(!ok.is_err());
    assert(ok.unwrap_ref() == SparseSmallEnum::y);

    assert(err.is_err());
    assert(!err.is_ok());

    Result<void, SparseSmallEnum> ok2(Ok());
    Result<void, SparseSmallEnum> err2(Err(SparseSmallEnum::z));

    assert(ok2.is_ok());
    assert(!ok2.is_err());

    assert(err2.is_err());
    assert(!err2.is_ok());
    assert(err2.unwrap_err_ref() == SparseSmallEnum::z);
}

static void test_enum_ok_void_err_big_uint32() {
    static_assert(sizeof(Result<BigEnum, void>) == sizeof(BigEnum));

    Result<BigEnum, void> ok(Ok(BigEnum::warning));
    Result<BigEnum, void> err(Err());

    assert(ok.is_ok());
    assert(!ok.is_err());
    assert(ok.unwrap_ref() == BigEnum::warning);

    assert(err.is_err());
    assert(!err.is_ok());

    auto chained = std::move(ok).and_then([](BigEnum value) {
        assert(value == BigEnum::warning);
        return Result<int, void>(Ok(77));
    });

    static_assert(std::is_same_v<decltype(chained), Result<int, void>>);
    assert(chained.is_ok());
    assert(std::move(chained).unwrap() == 77);

    Result<BigEnum, void> err2(Err());

    auto recovered = std::move(err2).or_else([] { return Result<BigEnum, int>(Err(404)); });

    assert(recovered.is_err());
    assert(std::move(recovered).unwrap_err() == 404);
}

static void test_void_ok_enum_err_big_uint32() {
    static_assert(sizeof(Result<void, BigEnum>) == sizeof(BigEnum));

    Result<void, BigEnum> ok(Ok());
    Result<void, BigEnum> err(Err(BigEnum::error));

    assert(ok.is_ok());
    assert(!ok.is_err());

    assert(err.is_err());
    assert(!err.is_ok());
    assert(err.unwrap_err_ref() == BigEnum::error);

    auto converted = std::move(err).or_else([](BigEnum value) {
        assert(value == BigEnum::error);
        return Result<void, int>(Err(500));
    });

    static_assert(std::is_same_v<decltype(converted), Result<void, int>>);
    assert(converted.is_err());
    assert(std::move(converted).unwrap_err() == 500);

    Result<void, BigEnum> ok2(Ok());

    auto mapped = std::move(ok2).map([] { return 1234; });

    static_assert(std::is_same_v<decltype(mapped), Result<int, BigEnum>>);
    assert(mapped.is_ok());
    assert(std::move(mapped).unwrap() == 1234);
}

static void test_enum_signed_big_int32() {
    static_assert(sizeof(Result<SignedBigEnum, void>) == sizeof(SignedBigEnum));
    static_assert(sizeof(Result<void, SignedBigEnum>) == sizeof(SignedBigEnum));

    Result<SignedBigEnum, void> ok(Ok(SignedBigEnum::positive));
    Result<SignedBigEnum, void> err(Err());

    assert(ok.is_ok());
    assert(!ok.is_err());
    assert(ok.unwrap_ref() == SignedBigEnum::positive);

    assert(err.is_err());
    assert(!err.is_ok());

    Result<void, SignedBigEnum> ok2(Ok());
    Result<void, SignedBigEnum> err2(Err(SignedBigEnum::negative));

    assert(ok2.is_ok());
    assert(!ok2.is_err());

    assert(err2.is_err());
    assert(!err2.is_ok());
    assert(err2.unwrap_err_ref() == SignedBigEnum::negative);
}

static void test_exception_unwrap_or_throw() {
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
    {
        Result<int, std::runtime_error> r(Ok(42));
        assert(std::move(r).unwrap_or_throw() == 42);
    }

    {
        Result<int, std::runtime_error> r(Err(std::runtime_error{"boom"}));

        bool caught = false;

        try {
            (void)std::move(r).unwrap_or_throw();
        } catch (const std::runtime_error& e) {
            caught = true;
            assert(std::string{e.what()} == "boom");
        }

        assert(caught);
    }

    {
        Result<int, void> r(Ok(9));
        assert(std::move(r).unwrap_or_throw() == 9);
    }

    {
        Result<void, void> r(Ok());
        std::move(r).unwrap_or_throw();
    }
#endif
}

static void test_operator_not_equal_bug_exposure() {
    {
        Result<int, std::string> r(Ok(1));

        // BUG: current implementation calls itself recursively:
        // bool operator!=(const wrapper::Ok<T>& ok) const { return *this != ok; }
        assert(r != Ok(2));
    }

    {
        Result<int, void> r(Ok(1));

        // Same recursion bug.
        assert(r != Ok(2));
    }

    {
        Result<void, std::string> r(Err(std::string{"x"}));

        // Same recursion bug.
        assert(r != Err(std::string{"y"}));
    }

    {
        Result<int, std::string> a(Ok(1));
        Result<int, std::string> b(Ok(2));

        // Same recursion bug.
        assert(a != b);
    }
}

static void compile_fail_non_default_sentinel_and_then_is_result_detection() {
    using R = Result<int, void, -1>;

    R r(Ok(1));

    // BUG: detail::is_result only specializes Result<T, E>, not Result<T, E, OS, ES>.
    // This should probably be accepted but currently fails the static_assert in and_then.
    auto out = std::move(r).and_then([](int x) { return R(Ok(x + 1)); });

    (void)out;
}

static void compile_fail_reference_unchecked_nonvoid_variant() {
    int x = 1;

    Result<int&, std::string> r(Ok(x));

    // BUG: Result<T, E>::unwrap_unchecked returns T but tries to return
    // std::move(ok_state().value). For T == int&, ok_state().value is int*, so this cannot convert
    // to int&.
    int& ref = std::move(r).unwrap_unchecked();

    (void)ref;
}

int main() {
    test_ok_err_wrapper_basics();

    test_result_void_void();

    test_result_t_void_ok_path();
    test_result_t_void_err_path();

    test_result_void_e_ok_path();
    test_result_void_e_err_path();

    test_result_t_e_ok_path();
    test_result_t_e_err_path();

    test_reference_ok_type();
    test_reference_err_type();

    test_move_only_ok_type();
    test_move_only_err_type();

    test_equality_positive_cases_only();

    test_sentinel_niche_result_t_void();
    test_sentinel_niche_result_void_e();

    test_enum_ok_void_err_small_uint8();
    test_void_ok_enum_err_small_uint8();
    test_enum_sparse_small_uint8();

    test_enum_ok_void_err_big_uint32();
    test_void_ok_enum_err_big_uint32();
    test_enum_signed_big_int32();

    test_exception_unwrap_or_throw();

    test_operator_not_equal_bug_exposure();

    return 0;
}