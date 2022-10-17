#include <filesystem>
#include <fmt/format.h>
#include <functional>
#include <openssl/md5.h>
#include <span>
#include <stdexcept>

#pragma once

using u64 = __uint64_t; //!< Unsigned 64-bit integer
using u32 = __uint32_t; //!< Unsigned 32-bit integer
using u16 = __uint16_t; //!< Unsigned 16-bit integer
using u8 = __uint8_t;   //!< Unsigned 8-bit integer

using Md5Hash = std::array<u8, MD5_DIGEST_LENGTH>; //!< An MD5 hash

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

    constexpr Section(size_t address, size_t size) : address{address}, length{address + size}, size{size} {}

    /**
     * @brief Get the range of bytes from the given data as an integer
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
     * @param newSection The data to replace the section with
     */
    constexpr void replace(std::span<u8> data, const std::span<u8> newSection) const {
        if (address > data.size_bytes() || size > data.size_bytes())
            throw exception("Invalid offset range while replacing: [0x{:X}, 0x{:X}], size: 0x{:X}", address, length, data.size_bytes());
        if (newSection.size_bytes() != size)
            throw exception("New section size 0x{:X} does not match old section size 0x{:X}", newSection.size_bytes(), size);

        std::copy(newSection.begin(), newSection.end(), data.begin() + address);
    }

    template <class C> constexpr void replace(std::span<u8> data, C newString) const {
        if (newString.size() < size)
            std::fill(data.begin() + address + newString.size(), data.begin() + address + size, 0); // 0 fill the remainder of the section
        else if (newString.size() > size)
            throw exception("String with length {:X} is bigger than section size {:X}", newString.size(), size);

        std::copy(newString.data(), newString.data() + newString.size(), data.begin() + address);
    }

    constexpr std::span<u8> bytesFrom(const std::span<u8> data) const {
        if (address > data.size_bytes() || size > data.size_bytes())
            throw exception("Invalid offset range: [{}, {}], size: {}", address, length, data.size_bytes());

        return data.subspan(address, length - address);
    }

    std::string_view stringFrom(const std::span<u8> data) const {
        return {reinterpret_cast<const char *>(bytesFrom(data).data()), size};
    }
};

namespace util {

/**
 * @brief Get the amount of digits in a number
 */
template <class C> constexpr u32 getDigits(C input) {
    u32 digits{};
    while (input > 0) {
        input /= 10;
        digits++;
    }
    return digits;
}

/**
 * @brief Replace all occurances of a span inside of another span
 */
template <typename T, class C> constexpr void ReplaceAll(std::span<T> data, std::span<T> find, C replace) {
    size_t index{};
    if (find.size_bytes() != replace.size())
        throw exception("Size of find does not match replace");

    while (true) {
        auto itr{std::search(data.begin() + index, data.end(), find.begin(), find.end())};
        if (itr == data.end())
            break;

        std::copy(replace.begin(), replace.end(), itr);
        index = itr - data.begin() + 1;
    }
}

const Md5Hash GenerateMd5(std::span<u8> input);

void Utf8ToUtf16(std::span<u8> chars, std::u16string_view text);

const std::string Utf16ToUtf8String(std::span<u8> text);

const std::string SecondsToTimeStamp(const time_t seconds);

/**
 * @brief Get an environment variable's value
 * @param defaultValue The value to return if the variable is not set
 */
const std::string GetEnvironmentVariable(std::string_view name, std::function<std::string()> defaultValue);

const std::string GetEnvironmentVariable(std::string_view name, std::string_view defaultValue = "");

/**
 * @brief Get the Steam ID based on the path to the savefile
 */
u64 GetSteamId(std::filesystem::path saveFilePath);

std::filesystem::path FindFileInSubDirectory(std::filesystem::path directory, std::string_view filename);

/**
 * @brief Get an std::filesystem::path's absolute path, used for logging
 */
const std::filesystem::path ToAbsolutePath(std::filesystem::path path);

/**
 * @brief The directory to write to at runtime
 */
std::filesystem::path CreateDataDirectory();

/**
 * @brief The directory to write a backup of the save file to at runtime
 */
std::filesystem::path CreateBackupDirectory();

/**
 * @brief Copy the savefile to the backup directory
 */
std::filesystem::path BackupSavefile(std::filesystem::path saveFilePath);

} // namespace util
