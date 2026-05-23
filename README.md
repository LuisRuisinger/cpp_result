# Result

A Rust-inspired `Result<T, E>` type for C++17 and later.

## Features

- `Ok<T>` / `Err<E>` result values
- `void` specializations
- lvalue reference support
- functional chaining via `map`, `map_err`, `and_then`, and `or_else`
- niche optimization for selected `Result<T, void>` / `Result<void, E>` cases

## Requirements

C++17 or later.

## Installation

```cmake
include(FetchContent)

FetchContent_Declare(
    result
    GIT_REPOSITORY https://github.com/LuisRuisinger/result.git
    GIT_TAG v0.1.0
)

FetchContent_MakeAvailable(result)

target_link_libraries(your_target PRIVATE lsr::result)
```

Include:

```cpp
#include <result/result.hpp>
```

## Basic Usage

```cpp
#include <iostream>
#include <string>

#include <result/result.hpp>

using namespace lsr::result;

Result<float, std::string> divide(int a, int b) {
    if (b == 0)
        return Err(std::string("division by zero"));

    return Ok(static_cast<float>(a) / b);
}

int main() {
    auto result = divide(10, 2);

    if (result.is_ok())
        std::cout << result.unwrap_ref() << '\n';

    return 0;
}
```

## Notes

Consuming methods such as `unwrap`, `unwrap_err`, `map`, and `and_then` are `&&`-qualified:

```cpp
auto value = std::move(result).unwrap();
```

Inspection methods such as `is_ok`, `is_err`, `unwrap_ref`, and `unwrap_err_ref` do not consume the result.

Headers under `result/detail/` are implementation details and are not part of the public API.

## License

MIT. See [LICENSE](LICENSE).
