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

std::filesystem::path findFileInSubDirectory(std::filesystem::path directory, std::string_view filename) {
    if (std::filesystem::exists(directory)) {
        for (auto &path : std::filesystem::directory_iterator(directory))
            if (path.is_directory()) {
                const auto filePath{path.path() / filename};
                if (std::filesystem::exists(filePath)) {
                    return filePath;
                }
            }
    }

    return {};
}

void utf8ToUtf16(std::span<u8> chars, std::u16string_view text) {
    std::memcpy(chars.data(), text.data(), std::min(text.size() * sizeof(char16_t), chars.size_bytes()));
    if (text.size() * sizeof(char16_t) > chars.size()) // Null terminate if the string if it isnt out of bounds
        *(reinterpret_cast<char16_t *>(chars.data()) + text.size()) = u'\0';
}

u64 getSteamId(std::filesystem::path saveFilePath) {
    u64 steamId;
    try {
        // Folder structure is 'EldenRing/<Steam ID>/ER0000.sl2'
        steamId = static_cast<u64>(std::stoull(saveFilePath.parent_path().filename().generic_string()));
    } catch (std::exception &e) {
        exception("Failed to parse Steam ID: {}", e.what());
    }

    return steamId;
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

std::filesystem::path backupAndRemoveSavefile(std::filesystem::path saveFilePath) {
    auto backupDir{util::makeBackupDirectory()};
    std::filesystem::path bakFilePath{saveFilePath.string() + ".bak"};
    if (std::filesystem::exists(saveFilePath))
        std::filesystem::copy(saveFilePath, backupDir / saveFilePath.filename());
    if (std::filesystem::exists(bakFilePath)) {
        std::filesystem::copy(bakFilePath, backupDir / bakFilePath.filename());
        std::filesystem::remove(bakFilePath); // If this differentiates from ER0000.sl2 the game will claim the savefile is corrupt
    }

    return backupDir;
}

} // namespace savepatcher::util
