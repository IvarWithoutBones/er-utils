#include "util.h"
#include <chrono>
#include <span>

namespace savepatcher::util {

const Md5Hash generateMd5(std::span<u8> input) {
    MD5_CTX sha256;
    Md5Hash hash{};

    MD5_Init(&sha256);
    MD5_Update(&sha256, input.data(), input.size_bytes());
    MD5_Final(hash.data(), &sha256);

    return hash;
}

const std::string secondsToTimeStamp(const time_t input) {
    constexpr static auto SecondsInHour{60};
    constexpr static auto MinutesInHour{SecondsInHour * 60};
    const auto hours{std::chrono::duration_cast<std::chrono::hours>(std::chrono::seconds(input))};
    const auto minutes{std::chrono::duration_cast<std::chrono::minutes>(std::chrono::seconds(input - (hours.count() * MinutesInHour)))};
    const auto seconds{std::chrono::duration_cast<std::chrono::seconds>(std::chrono::seconds(input - (hours.count() * MinutesInHour) - (minutes.count() * SecondsInHour)))};

    return fmt::format("{:02}:{:02}:{:02}", hours.count(), minutes.count(), seconds.count());
}

const std::string toAbsolutePath(std::filesystem::path path) {
    return std::filesystem::absolute(path).generic_string();
}

} // namespace savepatcher
