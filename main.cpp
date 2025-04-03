#include "result.hpp"



static_assert(Utils::is_convertible<void, void>::value);

Result<int, void> fun(int a) {
    return Ok(a);
}

int main() {
    Result<std::string, int> res = Ok(std::string { "hello world" });

    {
        std::string s = res.map_or_else(
                [](std::string &&s) -> Result<std::string, void> { return Ok("fuck " + s); },
                [](bool &&a) -> Result<std::string, void> { return Err(); }).unwrap();
        std::cout << s << std::endl;
    }

    {
        bool is_ok = res.map([]() -> void {}).map_err([]() -> void {}).is_ok();
        res.and_then([](std::string &&s) -> Result<void, int> { return Ok(); });
        // std::cout << s << std::endl;
    }

    Ok(2.0F) == Ok(2);
}
