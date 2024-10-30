#ifndef MSGPLUS_ORDERED_MAP_HPP
#define MSGPLUS_ORDERED_MAP_HPP

#include "flat_map.hpp"

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
