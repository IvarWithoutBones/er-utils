#include <span>
#include <stdexcept>
#include <vector>

#include <fmt/format.h>
#include <openssl/md5.h>

namespace savepatcher {

// Generate an MD5 hash of the given string using the OpenSSL library.
class md5sum {
  public:
    // Calculate the MD5 checksum of the given string.
    md5sum(const std::string &str) { checksum = generateMd5(str); }

    // Return the MD5 checksum as a vector of bytes.
    constexpr std::span<uint8_t> getBytes() { return std::span<uint8_t>(checksum.data(), checksum.size()); }

    // Get the generated MD5 checksum as a string.
    std::string getString() const {
        std::string result;

        for (auto &i : checksum)
            result += fmt::format("{:02x}", i);

        return result;
    }

  private:
    std::vector<uint8_t> checksum;
    MD5_CTX sha256;

    // Generate the MD5 checksum of the given string.
    std::vector<uint8_t> generateMd5(const std::string input) {
        uint8_t hash[MD5_DIGEST_LENGTH];
        std::vector<uint8_t> output;

        MD5_Init(&sha256);
        MD5_Update(&sha256, input.c_str(), input.size());
        MD5_Final(hash, &sha256);

        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            output.emplace_back(hash[i]);
        }

        return output;
    }
};

// Copied from
// https://github.com/skyline-emu/skyline/blob/8826c113b0bdd602ba302df7ad47febe751d45d1/app/src/main/cpp/skyline/common/base.h#L76
// A wrapper around std::runtime_error with {fmt} formatting
class exception : public std::runtime_error {
  public:
    template <typename S, typename... Args> constexpr auto Format(S formatString, Args &&...args) {
        return fmt::format(fmt::runtime(formatString), FmtCast(args)...);
    }

    template <typename T> constexpr auto FmtCast(T object) {
        if constexpr (std::is_pointer<T>::value)
            if constexpr (std::is_same<char,
                                       typename std::remove_cv<typename std::remove_pointer<T>::type>::type>::value)
                return reinterpret_cast<typename std::common_type<char *, T>::type>(object);
            else
                return reinterpret_cast<const uintptr_t>(object);
        else
            return object;
    }

    template <typename S, typename... Args>
    exception(const S &formatStr, Args &&...args) : runtime_error(Format(formatStr, args...)) {}
};

} // namespace savepatcher
