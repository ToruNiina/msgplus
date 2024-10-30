#ifndef MSGPLUS_WRITER_HPP
#define MSGPLUS_WRITER_HPP

#include <concepts>
#include <filesystem>
#include <fstream>

namespace msgplus
{

template<typename W>
concept Writer = requires(W& w) {
    {w.is_ok()} -> std::convertible_to<bool>;
    w.write_byte(std::declval<std::byte>());
    w.write_bytes(std::declval<const std::byte*>(), 8);
};

class file_writer
{
  public:

    file_writer(const std::filesystem::path& fpath)
        : file_(fpath)
    {}

    bool is_ok()  const noexcept {return file_.good();}

    void write_byte(std::byte b)
    {
        file_.put(std::bit_cast<char>(b));
    }
    void write_bytes(const std::byte* ptr, const std::size_t len)
    {
        if(ptr) {file_.write(reinterpret_cast<const char*>(ptr), len);}
    }

  private:

    std::ofstream file_;
};

static_assert(Writer<file_writer>);

} // msgplus
#endif//MSGPLUS_WRITER_HPP
