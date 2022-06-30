#include "util.h"
#include <chrono>
#include <functional>
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

const std::string getEnvironmentVariable(std::string_view name, std::function<std::string()> defaultValue) {
    return {(std::getenv(name.data()) != nullptr) ? std::getenv(name.data()) : defaultValue.operator()()};
}

const std::string getEnvironmentVariable(std::string_view name, std::string_view defaultValue) {
    return getEnvironmentVariable(name, [defaultValue]() { return defaultValue.data(); });
}

std::filesystem::path makeDataDirectory() {
    std::filesystem::path directory{getEnvironmentVariable("XDG_DATA_HOME", []() -> std::filesystem::path {
        auto home{getEnvironmentVariable("HOME")};
        if (!home.empty())
            return std::filesystem::path(home) / ".config";
        return std::filesystem::current_path();
    })};

    directory /= "er-saveutils";
    if (!std::filesystem::exists(directory))
        std::filesystem::create_directory(directory);
    return directory;
}

std::filesystem::path makeBackupDirectory() {
    auto directory{makeDataDirectory() / "backup"};
    const auto timePoint{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
    auto timeStamp{fmt::format("{}", std::ctime(&timePoint))}; // TODO: format this in a better way
    timeStamp.pop_back();                                      // remove newline, thanks ctime
    if (!std::filesystem::exists(directory))
        std::filesystem::create_directory(directory);

    directory /= timeStamp;
    if (!std::filesystem::exists(directory))
        std::filesystem::create_directory(directory);
    return directory;
}

} // namespace savepatcher::util
