#ifndef MSGPLUS_WRITE_HPP
#define MSGPLUS_WRITE_HPP

#include "value.hpp"
#include "writer.hpp"

#include <bit>

namespace msgplus
{

namespace detail
{
template<Writer W, typename T>
bool write_as_big_endian(W& writer, T x)
{
    constexpr auto N = sizeof(T);
    const auto as_bytes = reinterpret_cast<const std::byte*>(std::addressof(x));
    if constexpr(std::endian::native == std::endian::little)
    {
        for(std::size_t i=1; i<=N; ++i)
        {
            if( ! writer.write_byte(as_bytes[N-i]))
            {
                return false;
            }
        }
        return true;
    }
    else // native == big endian
    {
        return writer.write_bytes(as_bytes, N);
    }
}
} // detail

// forward decl
template<Writer W>
bool write(W& writer, const value& v);

template<Writer W>
bool write(W& writer, const nil_type&)
{
    return writer.write_byte(static_cast<std::byte>(0xC0));
}
template<Writer W>
bool write(W& writer, const bool_type& x)
{
    if(x)
    {
        return writer.write_byte(static_cast<std::byte>(0xC3));
    }
    else
    {
        return writer.write_byte(static_cast<std::byte>(0xC2));
    }
}
template<Writer W>
bool write(W& writer, const int_type& x)
{
    if(0 <= x)
    {
        if(x < 128)
        {
            return writer.write_byte(static_cast<std::byte>(x & 0xFF));
        }
        else if(x <= std::numeric_limits<std::uint8_t>::max())
        {
            if( ! writer.write_byte(static_cast<std::byte>(0xCC))) {return false;}
            return detail::write_as_big_endian(writer,
                    static_cast<std::uint8_t>(x));
        }
        else if(x <= std::numeric_limits<std::uint16_t>::max())
        {
            if( ! writer.write_byte(static_cast<std::byte>(0xCD))) {return false;}
            return detail::write_as_big_endian(writer,
                    static_cast<std::uint16_t>(x));
        }
        else if(x <= std::numeric_limits<std::uint32_t>::max())
        {
            if( ! writer.write_byte(static_cast<std::byte>(0xCE))) {return false;}
            return detail::write_as_big_endian(writer,
                    static_cast<std::uint32_t>(x));
        }
        else
        {
            if( ! writer.write_byte(static_cast<std::byte>(0xCF))) {return false;}
            return detail::write_as_big_endian(writer,
                    static_cast<std::uint64_t>(x));
        }
    }
    else
    {
        if(-32 <= x)
        {
            return detail::write_as_big_endian(writer,
                    static_cast<std::int8_t>(x));
        }
        else if(std::numeric_limits<std::int8_t>::min() <= x)
        {
            if( ! writer.write_byte(static_cast<std::byte>(0xD0))) {return false;}
            return detail::write_as_big_endian(writer,
                    static_cast<std::int8_t>(x));
        }
        else if(std::numeric_limits<std::int16_t>::min() <= x)
        {
            if( ! writer.write_byte(static_cast<std::byte>(0xD1))) {return false;}
            return detail::write_as_big_endian(writer,
                    static_cast<std::int16_t>(x));
        }
        else if(std::numeric_limits<std::int32_t>::min() <= x)
        {
            if( ! writer.write_byte(static_cast<std::byte>(0xD2))) {return false;}
            return detail::write_as_big_endian(writer,
                    static_cast<std::int32_t>(x));
        }
        else if(std::numeric_limits<std::int64_t>::min() <= x)
        {
            if( ! writer.write_byte(static_cast<std::byte>(0xD3))) {return false;}
            return detail::write_as_big_endian(writer, x);
        }
    }
    return false;
}
template<Writer W>
bool write(W& writer, const uint_type& x)
{
    if(x < 128)
    {
        return writer.write_byte(static_cast<std::byte>(x & 0xFF));
    }
    else if(x <= std::numeric_limits<std::uint8_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xCC))) {return false;}
        return detail::write_as_big_endian(writer,
                static_cast<std::uint8_t>(x));
    }
    else if(x <= std::numeric_limits<std::uint16_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xCD))) {return false;}
        return detail::write_as_big_endian(writer,
                static_cast<std::uint16_t>(x));
    }
    else if(x <= std::numeric_limits<std::uint32_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xCE))) {return false;}
        return detail::write_as_big_endian(writer,
                static_cast<std::uint32_t>(x));
    }
    else
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xCF))) {return false;}
        return detail::write_as_big_endian(writer,
                static_cast<std::uint64_t>(x));
    }
}
template<Writer W>
bool write(W& writer, const float32_type& x)
{
    if( ! writer.write_byte(static_cast<std::byte>(0xCA)))
    {
        return false;
    }
    return detail::write_as_big_endian(writer, x);
}
template<Writer W>
bool write(W& writer, const float64_type& x)
{
    if( ! writer.write_byte(static_cast<std::byte>(0xCB)))
    {
        return false;
    }
    return detail::write_as_big_endian(writer, x);
}
template<Writer W>
bool write(W& writer, const str_type& x)
{
    if(x.size() <= 31)
    {
        const std::uint8_t tag = 0b1010'0000 |
            static_cast<std::uint8_t>(x.size() & 0b0001'1111);
        if( ! writer.write_byte(static_cast<std::byte>(tag))) {return false;}
    }
    else if(x.size() <= std::numeric_limits<std::uint8_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xD9))) {return false;}
        if( ! detail::write_as_big_endian(writer, static_cast<std::uint8_t>(x.size()))) {return false;}
    }
    else if(x.size() <= std::numeric_limits<std::uint16_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xDA))) {return false;}
        if( ! detail::write_as_big_endian(writer, static_cast<std::uint16_t>(x.size()))) {return false;}
    }
    else if(x.size() <= std::numeric_limits<std::uint32_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xDB))) {return false;}
        if( ! detail::write_as_big_endian(writer, static_cast<std::uint32_t>(x.size()))) {return false;}
    }
    else
    {
        return false;
    }

    return writer.write_bytes(reinterpret_cast<const std::byte*>(x.data()), x.size());
}
template<Writer W>
bool write(W& writer, const bin_type& x)
{
    if(x.size() <= std::numeric_limits<std::uint8_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xC4))) {return false;}
        if( ! detail::write_as_big_endian(writer, static_cast<std::uint8_t>(x.size()))) {return false;}
    }
    else if(x.size() <= std::numeric_limits<std::uint16_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xC5))) {return false;}
        if( ! detail::write_as_big_endian(writer, static_cast<std::uint16_t>(x.size()))) {return false;}
    }
    else if(x.size() <= std::numeric_limits<std::uint32_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xC6))) {return false;}
        if( ! detail::write_as_big_endian(writer, static_cast<std::uint32_t>(x.size()))) {return false;}
    }
    else
    {
        return false;
    }
    return writer.write_bytes(x.data(), x.size());
}

template<Writer W>
bool write(W& writer, const array_type& x)
{
    if(x.size() <= 15)
    {
        const std::uint8_t tag = 0b1001'0000 |
            static_cast<std::uint8_t>(x.size() & 0b0000'1111);
        if( ! writer.write_byte(static_cast<std::byte>(tag))) {return false;}
    }
    else if(x.size() <= std::numeric_limits<std::uint16_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xDC))) {return false;}
        if( ! detail::write_as_big_endian(writer, static_cast<std::uint16_t>(x.size()))) {return false;}
    }
    else if(x.size() <= std::numeric_limits<std::uint32_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xDD))) {return false;}
        if( ! detail::write_as_big_endian(writer, static_cast<std::uint32_t>(x.size()))) {return false;}
    }
    else
    {
        return false;
    }

    for(const auto& elem : x)
    {
        if( ! write(writer, elem)) {return false;}
    }
    return true;
}

template<Writer W>
bool write(W& writer, const map_type& x)
{
    if(x.size() <= 15)
    {
        const std::uint8_t tag = 0b1000'0000 |
            static_cast<std::uint8_t>(x.size() & 0b0000'1111);
        if( ! writer.write_byte(static_cast<std::byte>(tag))) {return false;}
    }
    else if(x.size() <= std::numeric_limits<std::uint16_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xDC))) {return false;}
        if( ! detail::write_as_big_endian(writer, static_cast<std::uint16_t>(x.size()))) {return false;}
    }
    else if(x.size() <= std::numeric_limits<std::uint32_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xDD))) {return false;}
        if( ! detail::write_as_big_endian(writer, static_cast<std::uint32_t>(x.size()))) {return false;}
    }
    else
    {
        return false;
    }
    for(const auto& [k, v] : x)
    {
        if( ! write(writer, k)) {return false;}
        if( ! write(writer, v)) {return false;}
    }
    return true;
}

template<Writer W>
bool write(W& writer, const ext_type& x)
{
    if(x.second.size() == 1)
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xD4))) {return false;}
    }
    else if(x.second.size() == 2)
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xD5))) {return false;}
    }
    else if(x.second.size() == 4)
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xD6))) {return false;}
    }
    else if(x.second.size() == 8)
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xD7))) {return false;}
    }
    else if(x.second.size() == 16)
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xD8))) {return false;}
    }
    else if(x.second.size() <= std::numeric_limits<std::uint8_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xC7))) {return false;}
        if( ! detail::write_as_big_endian(writer, static_cast<std::uint8_t>(x.second.size()))) {return false;}
    }
    else if(x.second.size() <= std::numeric_limits<std::uint16_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xC8))) {return false;}
        if( ! detail::write_as_big_endian(writer, static_cast<std::uint16_t>(x.second.size()))) {return false;}
    }
    else if(x.second.size() <= std::numeric_limits<std::uint32_t>::max())
    {
        if( ! writer.write_byte(static_cast<std::byte>(0xC9))) {return false;}
        if( ! detail::write_as_big_endian(writer, static_cast<std::uint32_t>(x.second.size()))) {return false;}
    }
    else
    {
        return false;
    }
    if( ! writer.write_byte(static_cast<std::byte>(x.first))) {return false;}
    return writer.write_bytes(x.second.data(), x.second.size());
}

template<Writer W>
bool write(W& writer, const value& v)
{
    using enum type_t;
    switch(v.type())
    {
        case nil_t     : {return write(writer, v.as_nil    ());}
        case bool_t    : {return write(writer, v.as_bool   ());}
        case int_t     : {return write(writer, v.as_int    ());}
        case uint_t    : {return write(writer, v.as_uint   ());}
        case float32_t : {return write(writer, v.as_float32());}
        case float64_t : {return write(writer, v.as_float64());}
        case str_t     : {return write(writer, v.as_str    ());}
        case bin_t     : {return write(writer, v.as_bin    ());}
        case array_t   : {return write(writer, v.as_array  ());}
        case map_t     : {return write(writer, v.as_map    ());}
        case ext_t     : {return write(writer, v.as_ext    ());}
        default:         {return false;}
    }
}


} // msgplus
#endif//MSGPLUS_WRITE_HPP
