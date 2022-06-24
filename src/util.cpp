#include "util.h"
#include <chrono>
#include <span>

namespace savepatcher::util {

const Md5Hash GenerateMd5(std::span<u8> input) {
    MD5_CTX sha256;
    Md5Hash hash{};

    MD5_Init(&sha256);
    MD5_Update(&sha256, input.data(), input.size_bytes());
    MD5_Final(hash.data(), &sha256);

    return hash;
}

const std::string SecondsToTimeStamp(const time_t input) {
    constexpr static auto SecondsInHour{60};
    constexpr static auto MinutesInHour{SecondsInHour * 60};
    const auto hours{std::chrono::duration_cast<std::chrono::hours>(std::chrono::seconds(input))};
    const auto minutes{std::chrono::duration_cast<std::chrono::minutes>(std::chrono::seconds(input - (hours.count() * MinutesInHour)))};
    const auto seconds{std::chrono::duration_cast<std::chrono::seconds>(std::chrono::seconds(input - (hours.count() * MinutesInHour) - (minutes.count() * SecondsInHour)))};

    return fmt::format("{:02}:{:02}:{:02}", hours.count(), minutes.count(), seconds.count());
}

void replaceAll(std::span<u8> data, std::span<u8> find, std::span<u8> replace) {
    for (size_t i{}; i < data.size_bytes();) {
        auto it{std::search(data.begin() + i, data.end(), find.begin(), find.end())};
        if (it == data.end())
            break;

        i = it - data.begin();
        std::copy(replace.begin(), replace.end(), data.begin() + i);
    }
}

const std::string FormatHex(const std::span<u8> data) {
    return fmt::format("{:X}", fmt::join(data, ""));
}

const std::string toAbsolutePath(std::filesystem::path path) {
    return std::filesystem::absolute(path).generic_string();
}

} // namespace savepatcher
