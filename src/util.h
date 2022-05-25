#include <span>
#include <stdexcept>
#include <vector>

#include <fmt/format.h>
#include <openssl/md5.h>

namespace savepatcher {

using u16 = uint_fast16_t;                         //!< Unsigned 16-bit integer
using u8 = uint_fast8_t;                           //!< Unsigned 8-bit integer
using Md5Hash = std::array<u8, MD5_DIGEST_LENGTH>; //!< MD5 hash
constexpr static int bitsInByte = 0x8;             //!< Number of bits in a byte
constexpr static int bitsInShort = 0x10;           //!< Number of bits in a short

// Generate the MD5 checksum of the given string.
inline Md5Hash GenerateMd5(std::string_view input) {
    MD5_CTX sha256;
    Md5Hash hash{};

    MD5_Init(&sha256);
    MD5_Update(&sha256, input.data(), input.size());
    MD5_Final(hash.data(), &sha256);

    return hash;
}

// A wrapper around std::runtime_error with {fmt} formatting
// Copied from https://github.com/skyline-emu/skyline/blob/8826c113b0bdd602ba302df7ad47febe751d45d1/app/src/main/cpp/skyline/common/base.h#L76
class exception : public std::runtime_error {
  public:
    template <typename S, typename... Args> constexpr auto Format(S formatString, Args &&...args) {
        return fmt::format(fmt::runtime(formatString), FmtCast(args)...);
    }

    template <typename T> constexpr auto FmtCast(T object) {
        if constexpr (std::is_pointer<T>::value)
            if constexpr (std::is_same<char, typename std::remove_cv<typename std::remove_pointer<T>::type>::type>::value)
                return reinterpret_cast<typename std::common_type<char *, T>::type>(object);
            else
                return reinterpret_cast<const uintptr_t>(object);
        else
            return object;
    }

    template <typename S, typename... Args> exception(const S &formatStr, Args &&...args) : runtime_error(Format(formatStr, args...)) {}
};

} // namespace savepatcher
