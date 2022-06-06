#include "util.h"
#include <chrono>
#include <span>

namespace savepatcher {

util::Md5Hash util::generateMd5(std::span<u8> input) {
    MD5_CTX sha256;
    Md5Hash hash{};

    MD5_Init(&sha256);
    MD5_Update(&sha256, input.data(), input.size_bytes());
    MD5_Final(hash.data(), &sha256);

    return hash;
}

std::string util::secondsToTimestamp(const time_t input) {
    constexpr static auto MinutesInHour = 60;
    constexpr static auto SecondsInHour = MinutesInHour * 60;

    auto hours{std::chrono::duration_cast<std::chrono::hours>(std::chrono::seconds(input))};
    auto minutes{std::chrono::duration_cast<std::chrono::minutes>(std::chrono::seconds(input - (hours.count() * SecondsInHour)))};
    auto seconds{std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::seconds(input - (hours.count() * SecondsInHour) - (minutes.count() * MinutesInHour)))};

    return fmt::format("{:02}:{:02}:{:02}", hours.count(), minutes.count(), seconds.count());
}

std::string util::formatHex(const std::span<u8> data) {
    return fmt::format("{:X}", fmt::join(data, ""));
};

} // namespace savepatcher
