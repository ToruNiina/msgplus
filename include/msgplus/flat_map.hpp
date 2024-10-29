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

    reference operator[](key_type k)
    {
        const auto found = this->lower_bound(k);
        if(found->first == k)
        {
            return found->second;
        }
        const auto idx = std::distance(this->container_.begin(), found);
        this->container_.insert(found, std::make_pair(std::move(k), value_type{}));

        return this->container_.at(static_cast<size_type>(idx)).second;
    }
    template<typename K>
    std::enable_if_t<std::negation_v<std::is_same<std::remove_cvref_t<K>, key_type>>, reference>
    operator[](K&& k)
    {
        return this->operator[](key_type(std::forward<K>(k)));
    }

    template<typename K>
    reference at(const K& k)
    {
        const auto found = this->lower_bound(k);
        if(found->first == k)
        {
            return found->second;
        }
        else
        {
            throw std::out_of_range("flat_map::at()");
        }
    }
    template<typename K>
    const_reference at(const K& k) const
    {
        const auto found = this->lower_bound(k);
        if(found->first == k)
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
        if(found->first == k)
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
        if(found->first == k)
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
        return std::lower_bound(this->container_.begin(), this->container_.end(), k);
    }
    template<typename K>
    const_iterator lower_bound(const K& k) const
    {
        return std::lower_bound(this->container_.begin(), this->container_.end(), k);
    }

    template<typename K>
    iterator upper_bound(const K& k)
    {
        return std::upper_bound(this->container_.begin(), container_.end(), k);
    }
    template<typename K>
    const_iterator upper_bound(const K& k) const
    {
        return std::upper_bound(this->container_.begin(), container_.end(), k);
    }

    template<typename ... Args>
    std::pair<iterator, bool> emplace(Args&& ...args)
    {
        return this->insert(value_type(std::forward<Args>(args)...));
    }

    std::pair<iterator, bool> insert(value_type v)
    {
        const auto found = this->lower_bound(v.first);
        if(found->first == v.first)
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

    key_compare   key_comp()   const {return key_compare{};}
    value_compare value_comp() const {return value_compare{};}

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

template<typename K, typename V, typename A>
void swap(flat_map<K, V, A>& lhs, flat_map<K, V, A>& rhs)
{
    lhs.swap(rhs);
    return ;
}

} // msgplus
#endif// MSGPLUS_FLAT_MAP_HPP
