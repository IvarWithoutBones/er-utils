#include "util.h"
#include <chrono>
#include <span>

namespace savepatcher::util {

Md5Hash GenerateMd5(std::span<u8> input) {
    MD5_CTX sha256;
    Md5Hash hash{};

    MD5_Init(&sha256);
    MD5_Update(&sha256, input.data(), input.size_bytes());
    MD5_Final(hash.data(), &sha256);

    return hash;
}

std::string SecondsToTimeStamp(const time_t input) {
    constexpr static auto MinutesInHour = 60;
    constexpr static auto SecondsInHour = MinutesInHour * 60;

    auto hours{std::chrono::duration_cast<std::chrono::hours>(std::chrono::seconds(input))};
    auto minutes{std::chrono::duration_cast<std::chrono::minutes>(std::chrono::seconds(input - (hours.count() * SecondsInHour)))};
    auto seconds{std::chrono::duration_cast<std::chrono::seconds>(std::chrono::seconds(input - (hours.count() * SecondsInHour) - (minutes.count() * MinutesInHour)))};

    return fmt::format("{:02}:{:02}:{:02}", hours.count(), minutes.count(), seconds.count());
}

std::string FormatHex(const std::span<u8> data) {
    return fmt::format("{:X}", fmt::join(data, ""));
};

void replaceAll(std::span<u8> data, std::span<u8> find, std::span<u8> replace) {
    for (size_t i{}; i < data.size_bytes();) {
        auto it{std::search(data.begin() + i + 1, data.end(), find.begin(), find.end())};
        if (it == data.end())
            break;

        i = it - data.begin();
        fmt::print("Found {} at {:X}\n", util::FormatHex(find), i);
        std::copy(replace.begin(), replace.end(), data.begin() + i);
    }
}

} // namespace savepatcher
