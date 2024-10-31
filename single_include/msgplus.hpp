#ifndef MSGPLUS_WRITER_HPP
#define MSGPLUS_WRITER_HPP

#include <concepts>
#include <filesystem>
#include <fstream>

namespace msgplus
{

template<typename W>
concept Writer = requires(W& w) {
    {w.is_ok()}                                          -> std::convertible_to<bool>;
    {w.write_byte(std::declval<std::byte>())}            -> std::convertible_to<bool>;
    {w.write_bytes(std::declval<const std::byte*>(), 8)} -> std::convertible_to<bool>;
};

class file_writer
{
  public:

    file_writer(const std::filesystem::path& fpath)
        : file_(fpath)
    {}

    bool is_ok()  const noexcept {return file_.good();}

    bool write_byte(std::byte b)
    {
        file_.put(std::bit_cast<char>(b));
        return !file_.fail();
    }
    bool write_bytes(const std::byte* ptr, const std::size_t len)
    {
        if(ptr) {file_.write(reinterpret_cast<const char*>(ptr), len);}
        return !file_.fail();
    }

  private:

    std::ofstream file_;
};

static_assert(Writer<file_writer>);

} // msgplus
#endif//MSGPLUS_WRITER_HPP
#ifndef MSGPLUS_READER_HPP
#define MSGPLUS_READER_HPP

#include <array>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <vector>
#include <optional>

namespace msgplus
{

template<typename R>
concept Reader = requires(R& r) {
    {r.is_ok() }                 -> std::convertible_to<bool>;
    {r.is_eof()}                 -> std::convertible_to<bool>;
    {r.read_byte()}              -> std::same_as<std::optional<std::byte>>;
    {r.template read_bytes<8>()} -> std::same_as<std::optional<std::array<std::byte, 8>>>;
    {r.read_bytes(8)} -> std::same_as<std::optional<std::vector<std::byte>>>;
};

class file_reader
{
  public:

    file_reader(const std::filesystem::path& fpath)
        : file_(fpath)
    {}

    bool is_ok()  const noexcept {return file_.good();}
    bool is_eof() const noexcept {return file_.eof();}

    std::optional<std::byte> read_byte()
    {
        if(this->is_eof() || !this->is_ok()) {return std::nullopt;}

        const auto c = std::ifstream::traits_type::to_char_type(file_.get());
        if(file_.fail()) {return std::nullopt;}

        return std::bit_cast<std::byte>(c);
    }

    template<std::size_t N>
    std::optional<std::array<std::byte, N>> read_bytes()
    {
        if(this->is_eof() || !this->is_ok()) {return std::nullopt;}

        std::array<std::byte, N> retval;
        file_.read(reinterpret_cast<char*>(retval.data()), retval.size());
        if(file_.fail()) {return std::nullopt;}

        return retval;
    }

    std::optional<std::vector<std::byte>> read_bytes(std::size_t N)
    {
        if(this->is_eof() || !this->is_ok()) {return std::nullopt;}

        std::vector<std::byte> retval(N, static_cast<std::byte>(0));
        file_.read(reinterpret_cast<char*>(retval.data()), retval.size());
        if(file_.fail()) {return std::nullopt;}

        return retval;
    }

  private:

    std::ifstream file_;
};

static_assert(Reader<file_reader>);

} // msgplus
#endif//MSGPLUS_READER_HPP
#ifndef MSGPLUS_FLAT_MAP_HPP
#define MSGPLUS_FLAT_MAP_HPP

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace msgplus
{

template<typename Key, typename Value, typename Cmp = std::less<Key>,
         typename Allocator = std::allocator<std::pair<Key, Value>>>
class flat_map
{
  public:

    using key_type    = Key;
    using mapped_type = Value;
    using value_type  = std::pair<key_type, mapped_type>;
    using key_compare = Cmp;

    using allocator_type = typename std::allocator_traits<Allocator>::template
        rebind_alloc<value_type>;

    struct value_compare
    {
        [[no_unique_address]] key_compare cmp_;

        bool operator()(const value_type& lhs, const value_type& rhs) const
        {
            return cmp_(lhs.first, rhs.first);
        }
    };

    using container_type = std::vector<value_type, allocator_type>;

    using size_type       = typename container_type::size_type      ;
    using difference_type = typename container_type::difference_type;
    using reference       = typename container_type::reference;
    using const_reference = typename container_type::const_reference;
    using pointer         = typename container_type::pointer;
    using const_pointer   = typename container_type::const_pointer;
    using iterator        = typename container_type::iterator;
    using const_iterator  = typename container_type::const_iterator;

  public:

    flat_map() = default;
    ~flat_map() = default;
    flat_map(const flat_map&) = default;
    flat_map(flat_map&&)      = default;
    flat_map& operator=(const flat_map&) = default;
    flat_map& operator=(flat_map&&)      = default;

    flat_map(std::initializer_list<value_type> init,
             key_compare cmp = key_compare{},
             allocator_type alloc = allocator_type{})
        : cmp_(std::move(cmp)), container_(std::move(init), std::move(alloc))
    {
        std::sort(this->container_.begin(), this->container_.end(),
                  value_compare{cmp_});
    }
    template<typename InputIter>
    flat_map(InputIter begin, InputIter end,
             key_compare cmp = key_compare{},
             allocator_type alloc = allocator_type{})
        : cmp_(std::move(cmp)), container_(begin, end, std::move(alloc))
    {
        std::sort(this->container_.begin(), this->container_.end(),
                  value_compare{cmp_});
    }

    bool      empty()    const noexcept {return this->container_.empty()   ;}
    size_type size()     const noexcept {return this->container_.size()    ;}
    size_type max_size() const noexcept {return this->container_.max_size();}

    void clear() {return this->container_.clear();}

    iterator       begin()        noexcept {return this->container_.begin();}
    iterator       end()          noexcept {return this->container_.end();}
    const_iterator begin()  const noexcept {return this->container_.begin();}
    const_iterator end()    const noexcept {return this->container_.end();}
    const_iterator cbegin() const noexcept {return this->container_.cbegin();}
    const_iterator cend()   const noexcept {return this->container_.cend();}

    template<typename K>
    bool contains(const K& k)
    {
        return std::binary_search(this->container_.begin(), this->container_.end(), k);
    }

    template<typename K>
    size_type count(const K& k)
    {
        if(this->contains(k)) {return 1;} else {return 0;}
    }

    mapped_type& operator[](key_type k)
    {
        const auto found = this->lower_bound(k);
        if(found < this->end() && found->first == k)
        {
            return found->second;
        }
        const auto idx = std::distance(this->container_.begin(), found);
        this->container_.insert(found, std::make_pair(std::move(k), mapped_type{}));

        return this->container_.at(static_cast<size_type>(idx)).second;
    }

    template<typename K>
    mapped_type& at(const K& k)
    {
        const auto found = this->lower_bound(k);
        if(found < this->end() && found->first == k)
        {
            return found->second;
        }
        else
        {
            throw std::out_of_range("flat_map::at()");
        }
    }
    template<typename K>
    mapped_type const& at(const K& k) const
    {
        const auto found = this->lower_bound(k);
        if(found < this->end() && found->first == k)
        {
            return found->second;
        }
        else
        {
            throw std::out_of_range("flat_map::at()");
        }
    }

    template<typename K>
    iterator find(const K& k)
    {
        const auto found = this->lower_bound(k);
        if(found < this->end() && found->first == k)
        {
            return found;
        }
        else
        {
            return this->end();
        }
    }
    template<typename K>
    const_iterator find(const K& k) const
    {
        const auto found = this->lower_bound(k);
        if(found < this->end() && found->first == k)
        {
            return found;
        }
        else
        {
            return this->end();
        }
    }

    template<typename K>
    iterator lower_bound(const K& k)
    {
        return std::lower_bound(this->begin(), this->end(), k,
            [](const value_type& v, const key_type& k) {return v.first < k;});
    }
    template<typename K>
    const_iterator lower_bound(const K& k) const
    {
        return std::lower_bound(this->begin(), this->end(), k,
            [](const value_type& v, const key_type& k) {return v.first < k;});
    }

    template<typename K>
    iterator upper_bound(const K& k)
    {
        return std::upper_bound(this->begin(), this->end(), k,
            [](const value_type& v, const key_type& k) {return v.first < k;});
    }
    template<typename K>
    const_iterator upper_bound(const K& k) const
    {
        return std::upper_bound(this->begin(), this->end(), k,
            [](const value_type& v, const key_type& k) {return v.first < k;});
    }

    template<typename ... Args>
    std::pair<iterator, bool> emplace(Args&& ...args)
    {
        return this->insert(value_type(std::forward<Args>(args)...));
    }

    std::pair<iterator, bool> insert(value_type v)
    {
        const auto found = this->lower_bound(v.first);
        if(found < this->end() && found->first == v.first)
        {
            return std::make_pair(found, false);
        }
        else
        {
            const auto idx = std::distance(container_.begin(), found);
            container_.insert(std::move(v));
            return std::make_pair(std::next(this->container_.begin(), idx), true);
        }
    }

    void erase(const_iterator i)
    {
        this->container_.erase(i);
    }
    void erase(key_type k)
    {
        const auto found = this->find(k);
        if(found != this->end())
        {
            this->container_.erase(found);
        }
    }

    key_compare   key_comp()   const {return key_compare{};}
    value_compare value_comp() const {return value_compare{this->key_comp()};}

    bool operator==(const flat_map& rhs) const noexcept {return this->container_ == rhs.container_;}
    bool operator!=(const flat_map& rhs) const noexcept {return this->container_ != rhs.container_;}
    bool operator< (const flat_map& rhs) const noexcept {return this->container_ <  rhs.container_;}
    bool operator<=(const flat_map& rhs) const noexcept {return this->container_ <= rhs.container_;}
    bool operator> (const flat_map& rhs) const noexcept {return this->container_ >  rhs.container_;}
    bool operator>=(const flat_map& rhs) const noexcept {return this->container_ >= rhs.container_;}

    void swap(flat_map& other)
    {
        using std::swap;
        swap(other.cmp_,       this->cmp_);
        swap(other.container_, this->container_);
        return;
    }

  private:

    [[no_unique_address]] key_compare cmp_;
    container_type container_;
};

template<typename K, typename V, typename C, typename A>
void swap(flat_map<K, V, C, A>& lhs, flat_map<K, V, C, A>& rhs)
{
    lhs.swap(rhs);
    return ;
}

} // msgplus
#endif// MSGPLUS_FLAT_MAP_HPP
#ifndef MSGPLUS_ORDERED_MAP_HPP
#define MSGPLUS_ORDERED_MAP_HPP


#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

#include <cassert>

namespace msgplus
{

template<typename Key, typename Val, typename Cmp = std::less<Key>,
         typename Allocator = std::allocator<std::pair<Key, Val>>>
class ordered_map
{
  public:

    using key_type    = Key;
    using mapped_type = Val;
    using value_type  = std::pair<Key, Val>;
    using key_compare = Cmp;

    using allocator_type = typename std::allocator_traits<Allocator>::template
        rebind_alloc<value_type>;

    using container_type  = std::vector<value_type, Allocator>;
    using reference       = typename container_type::reference;
    using pointer         = typename container_type::pointer;
    using const_reference = typename container_type::const_reference;
    using const_pointer   = typename container_type::const_pointer;
    using iterator        = typename container_type::iterator;
    using const_iterator  = typename container_type::const_iterator;
    using size_type       = typename container_type::size_type;
    using difference_type = typename container_type::difference_type;

    using key_index_map = flat_map<key_type, size_type, key_compare, typename
        std::allocator_traits<allocator_type>::template rebind_alloc<std::pair<key_type, size_type>>
          >;

  public:

    ordered_map() = default;
    ~ordered_map() = default;
    ordered_map(const ordered_map&) = default;
    ordered_map(ordered_map&&)      = default;
    ordered_map& operator=(const ordered_map&) = default;
    ordered_map& operator=(ordered_map&&)      = default;

    template<typename InputIterator>
    ordered_map(InputIterator first, InputIterator last)
        :  container_(first, last)
    {
        this->construct_index();
    }

    ordered_map(std::initializer_list<value_type> v)
        : container_(std::move(v))
    {
        this->construct_index();
    }
    ordered_map& operator=(std::initializer_list<value_type> v)
    {
        this->container_ = std::move(v);
        this->construct_index();
        return *this;
    }

    iterator       begin()        noexcept {return container_.begin();}
    iterator       end()          noexcept {return container_.end();}
    const_iterator begin()  const noexcept {return container_.begin();}
    const_iterator end()    const noexcept {return container_.end();}
    const_iterator cbegin() const noexcept {return container_.cbegin();}
    const_iterator cend()   const noexcept {return container_.cend();}

    bool        empty()    const noexcept {return container_.empty();}
    std::size_t size()     const noexcept {return container_.size();}
    std::size_t max_size() const noexcept {return container_.max_size();}

    void clear() {container_.clear();}

    void push_back(value_type v)
    {
        if(this->contains(v.first))
        {
            throw std::out_of_range("ordered_map: value already exists");
        }
        this->key_index_[v.first] = this->container_.size();
        container_.push_back(std::move(v));
    }
    void emplace_back(key_type k, mapped_type v)
    {
        if(this->contains(k))
        {
            throw std::out_of_range("ordered_map: value already exists");
        }
        this->key_index_[k] = this->container_.size();
        container_.emplace_back(std::move(k), std::move(v));
    }
    void pop_back()
    {
        if(this->empty()) {return;}

        this->key_index_.erase(container_.back().first);
        container_.pop_back();
    }

    void insert(const_iterator pos, value_type kv)
    {
        if(this->contains(kv.first))
        {
            throw std::out_of_range("ordered_map: value already exists");
        }

        const auto index = std::distance(this->container_.cbegin(), pos);
        for(auto iter=pos; iter != this->cend(); ++iter)
        {
            assert(index <= this->key_index_.at(iter->first));
            this->key_index_.at(iter->first) += 1;
        }
        this->key_index_[kv.first] = index;

        container_.insert(pos, std::move(kv));
        return;
    }
    void emplace(const_iterator pos, key_type k, mapped_type v)
    {
        this->insert(pos, std::make_pair(std::move(k), std::move(v)));
        return;
    }

    std::size_t count(const key_type& key) const
    {
        return this->contains(key) ? 1 : 0;
    }
    bool contains(const key_type& key) const
    {
        return this->find(key) != this->end();
    }
    iterator find(const key_type& key) noexcept
    {
        const auto index = this->key_index_.find(key);
        if(index == this->key_index_.end())
        {
            return this->end();
        }
        assert(index->second < this->size());

        const auto found = std::next(this->begin(), index->second);
        assert(found->first == key);

        return found;
    }
    const_iterator find(const key_type& key) const noexcept
    {
        const auto index = this->key_index_.find(key);
        if(index == this->key_index_.end())
        {
            return this->end();
        }
        assert(index->second < this->size());

        const auto found = std::next(this->begin(), index->second);
        assert(found->first == key);

        return found;
    }

    mapped_type&       at(const key_type& k)
    {
        const auto iter = this->find(k);
        if(iter == this->end())
        {
            throw std::out_of_range("ordered_map: no such element");
        }
        return iter->second;
    }
    mapped_type const& at(const key_type& k) const
    {
        const auto iter = this->find(k);
        if(iter == this->end())
        {
            throw std::out_of_range("ordered_map: no such element");
        }
        return iter->second;
    }

    mapped_type& operator[](const key_type& k)
    {
        const auto iter = this->find(k);
        if(iter == this->end())
        {
            this->container_.emplace_back(k, mapped_type{});
            return this->container_.back().second;
        }
        else
        {
            return iter->second;
        }
    }

    mapped_type const& operator[](const key_type& k) const
    {
        const auto iter = this->find(k);
        if(iter == this->end())
        {
            throw std::out_of_range("ordered_map: no such element");
        }
        else
        {
            return iter->second;
        }
    }

    void swap(ordered_map& other)
    {
        container_.swap(other.container_);
    }

    bool operator==(const ordered_map& rhs) const noexcept {return this->container_ == rhs.container_;}
    bool operator!=(const ordered_map& rhs) const noexcept {return this->container_ != rhs.container_;}
    bool operator< (const ordered_map& rhs) const noexcept {return this->container_ <  rhs.container_;}
    bool operator<=(const ordered_map& rhs) const noexcept {return this->container_ <= rhs.container_;}
    bool operator> (const ordered_map& rhs) const noexcept {return this->container_ >  rhs.container_;}
    bool operator>=(const ordered_map& rhs) const noexcept {return this->container_ >= rhs.container_;}

  private:

    void construct_index()
    {
        this->key_index_.clear();
        for(size_type i=0; i<this->container_.size(); ++i)
        {
            this->key_index_[this->container_[i].first] = i;
        }
    }

  private:

    key_index_map  key_index_;
    container_type container_;
};

template<typename K, typename V, typename A>
void swap(ordered_map<K,V,A>& lhs, ordered_map<K,V,A>& rhs)
{
    lhs.swap(rhs);
    return;
}

} // msgplus
#endif // MSGPLUS_ORDERED_MAP_HPP
#ifndef MSGPLUS_VALUE_HPP
#define MSGPLUS_VALUE_HPP


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
#ifndef MSGPLUS_WRITE_HPP
#define MSGPLUS_WRITE_HPP


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
#ifndef MSGPLUS_READ_HPP
#define MSGPLUS_READ_HPP


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
#ifndef MSGPLUS_HPP
#define MSGPLUS_HPP

#define MSGPLUS_VERSION_MAJOR 0
#define MSGPLUS_VERSION_MINOR 1
#define MSGPLUS_VERSION_PATCH 0

#ifndef __cplusplus
#    error "__cplusplus is not defined"
#endif

#if defined(_MSVC_LANG) && defined(_MSC_VER) && 190024210 <= _MSC_FULL_VER
#  define MSGPLUS_CPLUSPLUS_STANDARD_VERSION _MSVC_LANG
#else
#  define MSGPLUS_CPLUSPLUS_STANDARD_VERSION __cplusplus
#endif

#if MSGPLUS_CPLUSPLUS_STANDARD_VERSION < 202002L
#    error "msgplus requires C++20 or later."
#endif

// IWYU pragma: begin_exports
// IWYU pragma: end_exports

#endif//MSGPLUS_HPP
