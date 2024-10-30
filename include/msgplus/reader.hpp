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
