#pragma once

#include <fmt/format.h>
#include <openssl/md5.h>
#include <span>
#include <stdexcept>

namespace savepatcher {

using u64 = __uint64_t; //!< Unsigned 64-bit integer
using u32 = __uint32_t; //!< Unsigned 32-bit integer
using u16 = __uint16_t; //!< Unsigned 16-bit integer
using u8 = __uint8_t;   //!< Unsigned 8-bit integer

namespace util {

using Md5Hash = std::array<u8, MD5_DIGEST_LENGTH>; //!< MD5 hash

/**
 * @brief Calculate the MD5 hash of a span of bytes
 * @param bytes The bytes to hash
 * @return The MD5 hash of the bytes
 */
Md5Hash generateMd5(std::span<u8> input);

/**
 * @brief Convert a number of seconds to a human-readable timestamp
 * @param seconds The number of seconds
 * @return The human-readable timestamp
 */
std::string secondsToTimestamp(const time_t input);

/**
 * @brief A convert a std::span to a hex string
 * @param span The span to convert
 * @return An uppercase hex string
 */
std::string formatHex(const std::span<u8> data);

} // namespace util

/**
 * @brief A wrapper around std::runtime_error with {fmt} formatting
 */
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
