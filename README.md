# cpp_result

A Rust-like `Result<T, E>` container compatible with C++11/14/17/20.

## Overview

`Result<T, E>` is a container that encapsulates either a successful value (`T`) or an error (`E`). 
It provides functional-style methods like `map`, `or_else`, `and_then`, and `unwrap` 
for error handling, transformation or logic branching. 

### Caveat
Any operation, transforming the container or 
the contained instances / types transfers ownership to the resulting type. For example

```cpp
#include "result.hpp"

Result<std::string void> a = Ok("hello world");

// safe; a is moved
std::cout << a.unwrap() << std::endl

// unsafe
std::cout << a.unwrap().size() << std::endl
```

## Example Usage

```cpp
#include <iostream>
#include <limits>

#include "result.hpp"

Result<float, std::string> divide(int a, int b) {
    if (b == 0) {
        return Err("Division by zero");
    }
    
    return Ok((float) a / b);
}

int main() {
    auto result = divide(10, 2)
       
        // transforms the value if the result contains Ok<float>
        .map([](float x) -> int { return x * 2; })
        
         // path is triggered if the result contains Err<std::string>
        .or_else([](std::string err) -> Result<float, void> {   
            std::cerr << "Error: " << err << std::endl;
            return Ok(std::numeric_limits<float>::infinity());
        });

    std::cout << result.unwrap() << std::endl; 
    return 0;
}
```