#ifndef MSGPLUS_READ_HPP
#define MSGPLUS_READ_HPP

#include "value.hpp"
#include "reader.hpp"

#include <bit>
#include <cstring>

namespace msgplus
{

namespace detail
{
template<typename T, Reader R>
std::optional<T> read_as_big_endian(R& reader)
{
    constexpr auto N = sizeof(T);
    T x;
    const auto as_bytes = reinterpret_cast<std::byte*>(std::addressof(x));
    if constexpr(std::endian::native == std::endian::little)
    {
        for(std::size_t i=1; i<=N; ++i)
        {
            if(const auto b = reader.read_byte())
            {
                as_bytes[N-i] = b.value();
            }
            else
            {
                return std::nullopt;
            }
        }
        return x;
    }
    else // native == big endian
    {
        if(const auto bs = reader.template read_bytes<N>())
        {
            std::memcpy(as_bytes, bs.value().data(), N);
            return x;
        }
        else
        {
            return std::nullopt;
        }
    }
}
} // detail

// forward decl
template<Reader R>
std::optional<value> read(R& reader);

namespace detail
{

template<Reader R>
std::optional<value> read_bin(R& reader, std::optional<std::size_t> len)
{
    if( ! len.has_value()) {return std::nullopt;}

    auto v = reader.read_bytes(len.value());
    if( ! v.has_value()) {return std::nullopt;}

    return value(std::move(v.value()));
}

template<Reader R>
std::optional<value> read_ext(R& reader, std::optional<std::size_t> len)
{
    if( ! len.has_value()) {return std::nullopt;}

    auto type = detail::read_as_big_endian<std::int8_t>(reader);
    if( ! type.has_value()) {return std::nullopt;}

    auto v = reader.read_bytes(len.value());
    if( ! v.has_value()) {return std::nullopt;}

    return value(std::make_pair(type.value(), std::move(v.value())));
}

template<Reader R>
std::optional<value> read_str(R& reader, std::optional<std::size_t> len)
{
    if( ! len.has_value()) {return std::nullopt;}

    auto v = reader.read_bytes(len.value());
    if( ! v.has_value()) {return std::nullopt;}

    std::string s(len.value(), '\0');
    std::transform(v.value().begin(), v.value().end(), s.begin(),
        [](const std::byte b) -> char { return std::bit_cast<char>(b); });

    return value(std::move(s));
}

template<Reader R>
std::optional<value> read_array(R& reader, std::optional<std::size_t> len)
{
    if( ! len.has_value()) {return std::nullopt;}

    std::vector<value> vs;
    vs.reserve(len.value());
    for(std::size_t i=0; i<len.value(); ++i)
    {
        auto elem = read(reader);
        if( ! elem.has_value()) {return std::nullopt;}
        vs.push_back(std::move(elem.value()));
    }
    return value(vs);
}

template<Reader R>
std::optional<value> read_map(R& reader, std::optional<std::size_t> len)
{
    if( ! len.has_value()) {return std::nullopt;}

    ordered_map<value, value> vs;
    for(std::size_t i=0; i<len.value(); ++i)
    {
        auto key = read(reader);
        if( ! key.has_value()) {return std::nullopt;}

        auto elem = read(reader);
        if( ! elem.has_value()) {return std::nullopt;}

        vs.emplace_back(std::move(key.value()), std::move(elem.value()));
    }
    return value(vs);
}

} // detail

template<Reader R>
std::optional<value> read(R& reader)
{
    if(const auto b = reader.read_byte())
    {
        const auto tag = static_cast<std::uint8_t>(b.value());
        if(tag <= 0x7F) // positive fixint
        {
            return value(tag);
        }
        else if(0xE0 <= tag) // negative fixint
        {
            const auto fixint = std::bit_cast<std::int8_t>(tag);
            return value(fixint);
        }
        else if((tag & 0b1110'0000) == 0b1010'0000) // fixstr
        {
            const auto len = tag & 0b0001'1111;
            return detail::read_str(reader, len);
        }
        else if((tag & 0b1111'0000) == 0b1001'0000) // fixarray
        {
            const auto len = tag & 0b0000'1111;
            return detail::read_array(reader, len);
        }
        else if((tag & 0b1111'0000) == 0b1000'0000) // fixmap
        {
            const auto len = tag & 0b0000'1111;
            return detail::read_map(reader, len);
        }

        switch(tag)
        {
            case 0xC0: {return value(nil_type{});}
            //   0xC1: {never used}
            case 0xC2: {return value(false);}
            case 0xC3: {return value(true);}
            case 0xC4: {return detail::read_bin(reader, detail::read_as_big_endian<std::uint8_t >(reader));}
            case 0xC5: {return detail::read_bin(reader, detail::read_as_big_endian<std::uint16_t>(reader));}
            case 0xC6: {return detail::read_bin(reader, detail::read_as_big_endian<std::uint32_t>(reader));}
            case 0xC7: {return detail::read_ext(reader, detail::read_as_big_endian<std::uint8_t >(reader));}
            case 0xC8: {return detail::read_ext(reader, detail::read_as_big_endian<std::uint16_t>(reader));}
            case 0xC9: {return detail::read_ext(reader, detail::read_as_big_endian<std::uint32_t>(reader));}
            case 0xCA: {return detail::read_as_big_endian<float32_type >(reader);}
            case 0xCB: {return detail::read_as_big_endian<float64_type >(reader);}
            case 0xCC: {return detail::read_as_big_endian<std::uint8_t >(reader);}
            case 0xCD: {return detail::read_as_big_endian<std::uint16_t>(reader);}
            case 0xCE: {return detail::read_as_big_endian<std::uint32_t>(reader);}
            case 0xCF: {return detail::read_as_big_endian<std::uint64_t>(reader);}
            case 0xD0: {return detail::read_as_big_endian<std::int8_t  >(reader);}
            case 0xD1: {return detail::read_as_big_endian<std::int16_t >(reader);}
            case 0xD2: {return detail::read_as_big_endian<std::int32_t >(reader);}
            case 0xD3: {return detail::read_as_big_endian<std::int64_t >(reader);}
            case 0xD4: {return detail::read_ext(reader,  1);}
            case 0xD5: {return detail::read_ext(reader,  2);}
            case 0xD6: {return detail::read_ext(reader,  4);}
            case 0xD7: {return detail::read_ext(reader,  8);}
            case 0xD8: {return detail::read_ext(reader, 16);}
            case 0xD9: {return detail::read_str(reader, detail::read_as_big_endian<std::uint8_t >(reader));}
            case 0xDA: {return detail::read_str(reader, detail::read_as_big_endian<std::uint16_t>(reader));}
            case 0xDB: {return detail::read_str(reader, detail::read_as_big_endian<std::uint32_t>(reader));}
            case 0xDC: {return detail::read_array(reader, detail::read_as_big_endian<std::uint16_t>(reader));}
            case 0xDD: {return detail::read_array(reader, detail::read_as_big_endian<std::uint32_t>(reader));}
            case 0xDE: {return detail::read_map(reader, detail::read_as_big_endian<std::uint16_t>(reader));}
            case 0xDF: {return detail::read_map(reader, detail::read_as_big_endian<std::uint32_t>(reader));}
            default: return std::nullopt;
        }
    }
    else
    {
        return std::nullopt;
    }
}

} // msgplus
#endif//MSGPLUS_READ_HPP
