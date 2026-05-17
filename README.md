# cpp_result

A Rust-inspired `Result<T, E>` type for C++17 and later, with niche optimization support,
reference semantics, and full `void` specializations.

## Requirements

C++17 or later.

## Overview

`Result<T, E>` is a type-safe container that holds either a successful value (`Ok<T>`) or an
error (`Err<E>`). It supports functional-style chaining via `map`, `map_err`, `and_then`,
`or_else`, and more.

### Template Signature

```cpp
template <typename T, typename E,
          auto OkSentinel  = tiny::UseDefaultValue,
          auto ErrSentinel = tiny::UseDefaultValue>
class Result;
```

Both `T` and `E` may independently be `void` or an lvalue reference type (`T&`).
Rvalue reference types (`T&&`) are not supported.

The optional `OkSentinel` / `ErrSentinel` parameters unlock **niche optimization** (see below).

### Specializations

| Specialization | Storage strategy |
|---|---|
| `Result<T, E>` | `std::variant<Ok<T>, Err<E>>` |
| `Result<T, void>` | `tiny::optional<T, OkSentinel>` — one object, no discriminator |
| `Result<void, E>` | `tiny::optional<E, ErrSentinel>` — one object, no discriminator |
| `Result<void, void>` | Single `bool` |

---

## Basic Usage

```cpp
#include "result.hpp"

Result<float, std::string> divide(int a, int b) {
    if (b == 0)
        return Err(std::string("division by zero"));
    return Ok(static_cast<float>(a) / b);
}

int main() {
    auto result = divide(10, 2);

    if (result.is_ok())
        std::cout << result.unwrap_ref() << '\n';   // non-consuming read
    else
        std::cerr << std::move(result).unwrap_err() << '\n';
}
```

### Ownership / move semantics

Consuming methods (`unwrap`, `unwrap_err`, `map`, `and_then`, …) require an rvalue — they are
`&&`-qualified and transfer ownership out of the `Result`. Inspection methods (`is_ok`,
`is_err`, `unwrap_ref`, `unwrap_err_ref`) are non-consuming and leave the container intact.

```cpp
Result<std::string, void> r = Ok(std::string("hello"));

std::cout << r.unwrap_ref() << '\n';       // ok — r still valid
std::cout << std::move(r).unwrap() << '\n'; // ok — r is consumed
// std::cout << r.unwrap_ref();            // undefined — r was moved from
```

---

## Niche Optimization

When one side of the `Result` is `void`, a sentinel value can be provided for the other side so
that the entire container fits in the size of `T` (or `E`) alone — no extra byte for a
discriminator.

### Automatic sentinels

Several types are handled automatically with no sentinel parameter required:

- **`float` / `double`** — a specific quiet NaN bit pattern that no platform uses as its default
  signaling or quiet NaN is reserved internally.
- **Pointers** — a non-canonical address that is invalid on all supported architectures.
- **`bool`** — the bit pattern `0xfe`, which is neither `0` nor `1`.
- **Enumerations** — the underlying integral range is probed at compile time (via
  `__PRETTY_FUNCTION__` / `__FUNCSIG__` inspection) to find a value that has no named enumerator.
  For 1-byte underlying types this is an exhaustive scan; for larger types it uses a deterministic
  pseudo-random search seeded from the type name. **No reserved enumerator is needed in the enum
  definition.**

```cpp
// float: automatic, sizeof == sizeof(float)
Result<float, void> r = Ok(3.14f);

// enum: automatic, no sentinel enumerator needed
enum class Status { Ok, Warn, Error };
Result<Status, void> s = Ok(Status::Warn);
```

### Explicit sentinels

You can also pin a specific sentinel value as a template argument. This is useful when you know a
value that can never appear in practice and want to document or enforce that constraint explicitly.

```cpp
// Reserve INT_MAX as the sentinel for an int payload.
Result<int, void, std::numeric_limits<int>::max()> r = Ok(42);
```

When using the default (`tiny::UseDefaultValue`) and the type has no known automatic sentinel,
storage falls back to `tiny::optional`'s default strategy (a `bool` discriminator alongside the
value).

---

## Constructing Results

```cpp
// Value types
Result<int, std::string> a = Ok(42);
Result<int, std::string> b = Err(std::string("oops"));

// void sides
Result<void, std::string> c = Ok();
Result<void, std::string> d = Err(std::string("oops"));
Result<void, void>        e = Ok();
Result<void, void>        f = Err();

// Reference types  — the Result stores a pointer internally; the caller must ensure
// the referenced object outlives the Result.
int x = 10;
Result<int&, std::string> g = Ok(x);
```

---

## API Reference

### Inspection (non-consuming)

| Method | Description |
|---|---|
| `is_ok() const` | Returns `true` if the result holds an `Ok` value. |
| `is_err() const` | Returns `true` if the result holds an `Err` value. |
| `unwrap_ref() &` | Returns a reference to the `Ok` value; terminates on error. |
| `unwrap_ref() const&` | Const overload of the above. |
| `unwrap_err_ref() &` | Returns a reference to the `Err` value; terminates on ok. |
| `unwrap_err_ref() const&` | Const overload of the above. |

### Consuming (require `std::move(result).method()`)

| Method | Description |
|---|---|
| `unwrap() &&` | Returns the `Ok` value; terminates on error. |
| `unwrap_err() &&` | Returns the `Err` value; terminates on ok. |
| `unwrap_unchecked() &&` | Returns the `Ok` value without any check. |
| `unwrap_err_unchecked() &&` | Returns the `Err` value without any check. |
| `unwrap_or(T fallback) &&` | Returns the `Ok` value, or `fallback` on error. |
| `unwrap_or_default() &&` | Returns the `Ok` value, or `T{}` on error. |
| `unwrap_or_throw() &&` | Returns the `Ok` value, or throws `bad_result_access` (or the `Err` value if it derives from `std::exception`). Requires exception support. |
| `expect(message) &&` | Returns the `Ok` value; terminates with `message` on error. |

### Transformation (consuming, return a new `Result`)

| Method | Description |
|---|---|
| `map(fn) &&` | Applies `fn` to the `Ok` value, returning `Result<Ret, E>`. Passes the error through unchanged. |
| `map_err(fn) &&` | Applies `fn` to the `Err` value, returning `Result<T, ErrRet>`. Passes the ok value through unchanged. |
| `map_or(fn, fallback) &&` | Returns `fn(ok_value)` or `fallback` on error. |
| `map_or_else(fn, other) &&` | Returns `fn(ok_value)` on ok, or `other(err_value)` on error. |
| `and_then(fn) &&` | Applies `fn` to the `Ok` value; `fn` must return a `Result` with the same `E`. Short-circuits on error. |
| `or_else(fn) &&` | Applies `fn` to the `Err` value; `fn` must return a `Result` with the same `T`. Short-circuits on ok. |

---

## Chaining Example

```cpp
#include <iostream>
#include <limits>
#include "result.hpp"

Result<float, std::string> divide(int a, int b) {
    if (b == 0)
        return Err(std::string("division by zero"));
    return Ok(static_cast<float>(a) / b);
}

int main() {
    float value = divide(10, 0)
        .map([](float x) { return x * 2.0f; })
        .or_else([](std::string err) -> Result<float, std::string> {
            std::cerr << "Error: " << err << '\n';
            return Ok(std::numeric_limits<float>::infinity());
        })
        .unwrap_or(0.0f);

    std::cout << value << '\n';   // inf
}
```

---

## Namespace

Define `RESULT_NAMESPACE` before including the header to wrap everything in `lsr::result`:

```cpp
#define RESULT_NAMESPACE
#include "result.hpp"

using namespace lsr::result;
```

---

## Comparison

`Result` values support `==` and `!=` against `wrapper::Ok<T>`, `wrapper::Err<E>`, and other
`Result` instances with compatible types.

```cpp
Result<int, std::string> r = Ok(42);
assert(r == Ok(42));
assert(r != Err(std::string("x")));
```

---

## Error Handling Policy

When an invalid access is detected (e.g. calling `unwrap()` on an `Err` result), the library
prints a message to `stderr` and calls `std::terminate`. There is no silent undefined behaviour
on checked accessors.

Unchecked accessors (`unwrap_unchecked`, `unwrap_err_unchecked`) skip the check entirely — use
them only when you have already verified the state with `is_ok()` / `is_err()`.