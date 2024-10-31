# msgplus

Tiny (~1500 lines), standalone (depending only on the STL) msgpack implementation for C++20 or later

## Examples

```cpp
#include <iostream>

int main()
{
    namespace msg = msgplus;

    msg::value v(msg::map_type{
            {"foo", 42},
            {"bar", true},
            {"π", 3.14},
            {"array", msg::array_type{1,2,3,4,5}}
        });

    std::cout << v.as_map().at("foo"  ).as_int()                  << std::endl;
    std::cout << v.as_map().at("bar"  ).as_bool()                 << std::endl;
    std::cout << v.as_map().at("π"    ).as_float64()              << std::endl;
    std::cout << v.as_map().at("array").as_array().at(0).as_int() << std::endl;

    {
        msg::file_writer w("example.msg");

        const auto result = msg::write(w, v);
        assert(result);
    } // call dtor to flush file_writer

    msg::file_reader r("example.msg");

    const auto u = msg::read(r).value();

    std::cout << u.as_map().at("foo"  ).as_uint()                  << std::endl;
    std::cout << u.as_map().at("bar"  ).as_bool()                  << std::endl;
    std::cout << u.as_map().at("π"    ).as_float64()               << std::endl;
    std::cout << u.as_map().at("array").as_array().at(0).as_uint() << std::endl;

    return 0;
}
```

## Integration

add `path/to/this/repo/include/` to your include path.

## Licensing terms

This product is licensed under the terms of the [MIT License](LICENSE).

- Copyright (c) 2024-now Toru Niina

All rights reserved.
