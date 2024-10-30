#ifndef MSGPLUS_VALUE_HPP
#define MSGPLUS_VALUE_HPP

#include "ordered_map.hpp"

#include <concepts>
#include <variant>
#include <vector>

#include <cstdint>

namespace msgplus
{

enum class type_t : std::uint8_t
{
    nil_t     =  0,
    bool_t    =  1,
    int_t     =  2,
    uint_t    =  3,
    float32_t =  4,
    float64_t =  5,
    str_t     =  6,
    bin_t     =  7,
    array_t   =  8,
    map_t     =  9,
    ext_t     = 10,
};

class value
{
  public:

    using nil_type     = std::monostate;
    using bool_type    = bool;
    using int_type     = std::int64_t;
    using uint_type    = std::uint64_t;
    using float32_type = float;
    using float64_type = double;
    using str_type     = std::string;
    using bin_type     = std::vector<std::byte>;
    using array_type   = std::vector<value>;
    using map_type     = ordered_map<value, value>;
    using ext_type     = std::pair<std::int8_t, std::vector<std::byte>>;

  public:

    value(): value_(nil_type{}) {}
    ~value() = default;
    value(const value&) = default;
    value(value&&)      = default;
    value& operator=(const value&) = default;
    value& operator=(value&&)      = default;

    value(nil_type     v): value_(std::move(v)) {}
    value(bool_type    v): value_(std::move(v)) {}
    template<std::signed_integral T>
    value(T v): value_(int_type(std::move(v))) {}
    template<std::unsigned_integral T>
    value(T v): value_(uint_type(std::move(v))) {}
    value(float32_type v): value_(std::move(v)) {}
    value(float64_type v): value_(std::move(v)) {}
    value(str_type     v): value_(std::move(v)) {}
    template<std::size_t N>
    value(const char (&v)[N]): value_(str_type(v)) {}
    value(const char* v): value_(str_type(v)) {}
    value(bin_type     v): value_(std::move(v)) {}
    value(array_type   v): value_(std::move(v)) {}
    value(map_type     v): value_(std::move(v)) {}
    value(ext_type     v): value_(std::move(v)) {}

    value& operator=(nil_type     v) { value_ = std::move(v); return *this;}
    value& operator=(bool_type    v) { value_ = std::move(v); return *this;}
    template<std::signed_integral T>
    value& operator=(T v) { value_ = int_type(std::move(v)); return *this;}
    template<std::unsigned_integral T>
    value& operator=(T v) { value_ = uint_type(std::move(v)); return *this;}
    value& operator=(float32_type v) { value_ = std::move(v); return *this;}
    value& operator=(float64_type v) { value_ = std::move(v); return *this;}
    template<std::size_t N>
    value& operator=(const char (&v)[N]){ value_ = str_type(v); return *this;}
    value& operator=(const char* v)     { value_ = str_type(v); return *this;}
    value& operator=(bin_type     v) { value_ = std::move(v); return *this;}
    value& operator=(array_type   v) { value_ = std::move(v); return *this;}
    value& operator=(map_type     v) { value_ = std::move(v); return *this;}
    value& operator=(ext_type     v) { value_ = std::move(v); return *this;}

    type_t type() const noexcept {return static_cast<type_t>(value_.index());}

    bool is_nil    () const noexcept {return value_.index() ==  0;}
    bool is_bool   () const noexcept {return value_.index() ==  1;}
    bool is_int    () const noexcept {return value_.index() ==  2;}
    bool is_uint   () const noexcept {return value_.index() ==  3;}
    bool is_float32() const noexcept {return value_.index() ==  4;}
    bool is_float64() const noexcept {return value_.index() ==  5;}
    bool is_str    () const noexcept {return value_.index() ==  6;}
    bool is_bin    () const noexcept {return value_.index() ==  7;}
    bool is_array  () const noexcept {return value_.index() ==  8;}
    bool is_map    () const noexcept {return value_.index() ==  9;}
    bool is_ext    () const noexcept {return value_.index() == 10;}

    nil_type    & as_nil    () {return std::get< 0>(value_);}
    bool_type   & as_bool   () {return std::get< 1>(value_);}
    int_type    & as_int    () {return std::get< 2>(value_);}
    uint_type   & as_uint   () {return std::get< 3>(value_);}
    float32_type& as_float32() {return std::get< 4>(value_);}
    float64_type& as_float64() {return std::get< 5>(value_);}
    str_type    & as_str    () {return std::get< 6>(value_);}
    bin_type    & as_bin    () {return std::get< 7>(value_);}
    array_type  & as_array  () {return std::get< 8>(value_);}
    map_type    & as_map    () {return std::get< 9>(value_);}
    ext_type    & as_ext    () {return std::get<10>(value_);}

    nil_type     const& as_nil    () const {return std::get< 0>(value_);}
    bool_type    const& as_bool   () const {return std::get< 1>(value_);}
    int_type     const& as_int    () const {return std::get< 2>(value_);}
    uint_type    const& as_uint   () const {return std::get< 3>(value_);}
    float32_type const& as_float32() const {return std::get< 4>(value_);}
    float64_type const& as_float64() const {return std::get< 5>(value_);}
    str_type     const& as_str    () const {return std::get< 6>(value_);}
    bin_type     const& as_bin    () const {return std::get< 7>(value_);}
    array_type   const& as_array  () const {return std::get< 8>(value_);}
    map_type     const& as_map    () const {return std::get< 9>(value_);}
    ext_type     const& as_ext    () const {return std::get<10>(value_);}

    nil_type    * try_nil    () {return std::get_if< 0>(std::addressof(value_));}
    bool_type   * try_bool   () {return std::get_if< 1>(std::addressof(value_));}
    int_type    * try_int    () {return std::get_if< 2>(std::addressof(value_));}
    uint_type   * try_uint   () {return std::get_if< 3>(std::addressof(value_));}
    float32_type* try_float32() {return std::get_if< 4>(std::addressof(value_));}
    float64_type* try_float64() {return std::get_if< 5>(std::addressof(value_));}
    str_type    * try_str    () {return std::get_if< 6>(std::addressof(value_));}
    bin_type    * try_bin    () {return std::get_if< 7>(std::addressof(value_));}
    array_type  * try_array  () {return std::get_if< 8>(std::addressof(value_));}
    map_type    * try_map    () {return std::get_if< 9>(std::addressof(value_));}
    ext_type    * try_ext    () {return std::get_if<10>(std::addressof(value_));}

    nil_type     const* try_nil    () const {return std::get_if< 0>(std::addressof(value_));}
    bool_type    const* try_bool   () const {return std::get_if< 1>(std::addressof(value_));}
    int_type     const* try_int    () const {return std::get_if< 2>(std::addressof(value_));}
    uint_type    const* try_uint   () const {return std::get_if< 3>(std::addressof(value_));}
    float32_type const* try_float32() const {return std::get_if< 4>(std::addressof(value_));}
    float64_type const* try_float64() const {return std::get_if< 5>(std::addressof(value_));}
    str_type     const* try_str    () const {return std::get_if< 6>(std::addressof(value_));}
    bin_type     const* try_bin    () const {return std::get_if< 7>(std::addressof(value_));}
    array_type   const* try_array  () const {return std::get_if< 8>(std::addressof(value_));}
    map_type     const* try_map    () const {return std::get_if< 9>(std::addressof(value_));}
    ext_type     const* try_ext    () const {return std::get_if<10>(std::addressof(value_));}

    bool operator==(const value& rhs) const noexcept {return this->value_ == rhs.value_;}
    bool operator!=(const value& rhs) const noexcept {return this->value_ != rhs.value_;}
    bool operator< (const value& rhs) const noexcept {return this->value_ <  rhs.value_;}
    bool operator<=(const value& rhs) const noexcept {return this->value_ <= rhs.value_;}
    bool operator> (const value& rhs) const noexcept {return this->value_ >  rhs.value_;}
    bool operator>=(const value& rhs) const noexcept {return this->value_ >= rhs.value_;}

    void swap(value& other) noexcept
    {
        using std::swap;
        swap(this->value_, other.value_);
    }

  private:

    std::variant<
        nil_type    ,
        bool_type   ,
        int_type    ,
        uint_type   ,
        float32_type,
        float64_type,
        str_type    ,
        bin_type    ,
        array_type  ,
        map_type    ,
        ext_type
    > value_;
};

using nil_type     = value::nil_type    ;
using bool_type    = value::bool_type   ;
using int_type     = value::int_type    ;
using uint_type    = value::uint_type   ;
using float32_type = value::float32_type;
using float64_type = value::float64_type;
using str_type     = value::str_type    ;
using bin_type     = value::bin_type    ;
using array_type   = value::array_type  ;
using map_type     = value::map_type    ;
using ext_type     = value::ext_type    ;

inline void swap(value& lhs, value& rhs) noexcept
{
    lhs.swap(rhs);
    return;
}

} // msgplus
#endif // MSGPLUS_VALUE_HPP
