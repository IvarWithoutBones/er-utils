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

using Md5Hash = std::array<u8, MD5_DIGEST_LENGTH>; //!< MD5 hash

/**
 * @brief Calculate the MD5 hash of a span of bytes
 * @param bytes The bytes to hash
 * @return The MD5 hash of the bytes
 */
Md5Hash GenerateMd5(std::span<u8> input);

/**
 * @brief Convert a number of seconds to a human-readable timestamp
 * @param seconds The number of seconds
 * @return The human-readable timestamp
 */
std::string SecondsToTimeStamp(const time_t input);

/**
 * @brief A convert a std::span to a hex string
 * @param span The span to convert
 * @return An uppercase hex string
 */
std::string FormatHex(const std::span<u8> data);

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

/**
 * @brief An object representing a range of bytes in a file with some utility functions
 */
struct Section {
    size_t address;
    size_t length;
    size_t size;

    constexpr Section(size_t address, size_t size) : address{address}, size{size}, length{address + size} {}

    /**
     * @brief Get the range of bytes from the given data as an integer
     * @param data The data to return the range from
     * @return A number containing the range of bytes from the given offset. This can be one of u8, u16, u32 or u64
     */
    template <typename T> constexpr T castInteger(const std::span<u8> data) const {
        // clang-format off
        using Type = std::conditional_t<sizeof(T) % sizeof(u64) == 0, u64,
                     std::conditional_t<sizeof(T) % sizeof(u32) == 0, u32,
                     std::conditional_t<sizeof(T) % sizeof(u16) == 0, u16, u8>>>;
        // clang-format on

        return static_cast<T>(*reinterpret_cast<const Type *>(data.data() + address));
    }

    /**
     * @brief Replace a section inside of the save file
     * @param data The data to modify
     * @param newSection The new data to replace the old section with
     */
    constexpr void replace(std::span<u8> data, const std::span<u8> newSection) const {
        if (address < 0 || address > data.size_bytes() || size > data.size_bytes())
            throw exception("Invalid offset range while replacing: [{}, {}], size: {}", address, length, data.size_bytes());
        if (newSection.size_bytes() != size)
            throw exception("New section size {} does not match old section size {}", newSection.size_bytes(), size);

        std::copy(newSection.begin(), newSection.end(), data.begin() + address);
    }

    /**
     * @brief Get the range of bytes from the given data
     * @param data The data to return the range from
     * @return A span containing the range of bytes from the given offset
     */
    constexpr std::span<u8> bytesFrom(const std::span<u8> data) const {
        if (address < 0 || address > data.size_bytes() || size > data.size_bytes())
            throw exception("Invalid offset range: [{}, {}], size: {}", address, length, data.size_bytes());

        return data.subspan(address, length - address);
    }

    /**
     * @brief Get the range of bytes from the given data as a string
     * @param data The data to return the range from
     * @return A string containing the range of bytes from the given offset
     */
    std::string charsFrom(const std::span<u8> data) const {
        auto section{bytesFrom(data)};
        auto chars{static_cast<u8 *>(section.data())};
        return {chars, chars + section.size_bytes()};
    };
};

/**
 * @brief A wrapper around Section that abstracts away different save slot addresses
 */
struct SlotSection : public Section {
    constexpr static size_t SlotLength = 0x24C; //!< The length of a save slot

    constexpr SlotSection(size_t address, size_t size, size_t slot) : Section{address + (slot * SlotLength), size} {}
};

} // namespace savepatcher
