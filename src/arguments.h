#include "util.h"
#include <filesystem>
#include <fmt/core.h>
#include <ranges>
#include <string>
#include <vector>

namespace CommandLineArguments {
using namespace savepatcher; // for exception

struct ArgumentBase {
    constexpr virtual ~ArgumentBase() = default;
    constexpr ArgumentBase() = default;
    constexpr ArgumentBase(const ArgumentBase &) = default;
};

/**
 * @brief The container type for static command line arguments
 */
template <typename T> struct Argument : ArgumentBase {
    const std::string_view name{};
    const std::string_view description;
    const std::string_view briefDescription;
    bool set{false};
    T value{};

    constexpr Argument() = default;
    constexpr Argument(std::string_view name) : name(name) {}
    constexpr Argument(std::string_view name, std::string_view description) : name(name), description(description) {}
    constexpr Argument(std::string_view name, std::string_view briefDescription, std::string_view description) : name(name), description(description), briefDescription{briefDescription} {}
};

/**
 * @brief A command line argument parser
 */
class ArgumentParser {
  private:
    const std::vector<std::string_view> rawArguments;     //!< The arguments passed to the program
    std::vector<std::shared_ptr<ArgumentBase>> arguments; //!< The container with all expected arguments
    std::vector<std::string_view> argumentNames;          //!< The arguments expected by the program. Ideally these would be derived from the argument container, but im not sure how since it contains templates
    std::string_view programName;                         //!< The name of the program
    std::string briefUsageDescription;                    //!< A one line description of the arguments
    std::string usageDescription;                         //!< A full description of all arguments

    /**
     * @brief Append an argument to the argument container and parse it if it is set
     */
    template <typename T> constexpr void init(Argument<T> &arg) {
        // TODO: dont assume formatting like this
        if (!arg.briefDescription.empty())
            briefUsageDescription += fmt::format("{} {} ", arg.name, arg.briefDescription);
        if (!arg.description.empty()) {
            if (!arg.briefDescription.empty())
                usageDescription += fmt::format("    {} {}: {}\n", arg.name, arg.briefDescription, arg.description);
            else
                usageDescription += fmt::format("    {}: {}\n", arg.name, arg.description);
        }

        if (isSet(arg.name)) {
            parse(arg);
            arg.set = true;
        }

        argumentNames.emplace_back(arg.name);
        arguments.emplace_back(std::make_shared<Argument<T>>(arg));
    }

    constexpr bool isSet(std::string_view argumentName) const {
        return std::find(rawArguments.begin(), rawArguments.end(), argumentName) == rawArguments.end() ? false : true;
    }

    /**
     * @brief Get the value of the argument after the given name
     */
    constexpr std::string_view getNextArgument(std::string_view argumentName) {
        const auto nextArgPos{static_cast<size_t>(std::distance(rawArguments.begin(), std::find(rawArguments.begin(), rawArguments.end(), argumentName)) + 1)};
        if (nextArgPos >= rawArguments.size())
            throw exception("Missing argument value for '{}'", argumentName);

        argumentNames.emplace_back(rawArguments[nextArgPos]);
        return rawArguments[nextArgPos];
    }

    void parse(Argument<int> &arg) {
        try {
            arg.value = std::stoi(getNextArgument(arg.name).data());
        } catch (const std::exception) {
            throw exception("Invalid value for '{}'", arg.name);
        }
    }

    void parse(Argument<std::tuple<int, std::string_view>> &arg) {
        auto nextArg{getNextArgument(arg.name).data()};
        std::string_view nextNextArg{getNextArgument(nextArg)};
        try {
            arg.value = std::make_tuple(std::stoi(nextArg), nextNextArg);
        } catch (const std::exception) {
            throw exception("Invalid value for '{}'", arg.name);
        }
    }

    void parse(Argument<u64> &arg) {
        try {
            arg.value = std::stoull(getNextArgument(arg.name).data());
        } catch (const std::exception) {
            throw exception("Invalid value for '{}'", arg.name);
        }
    }

    constexpr void parse(Argument<bool> &arg) const {
        arg.value = true;
    }

    constexpr void parse(Argument<std::string_view> &arg) {
        arg.value = getNextArgument(arg.name);
    }

  public:
    ArgumentParser(int argc, char **argv) : rawArguments{argv, argv + argc}, programName{argv[0]} {}

    template <typename T> constexpr void add(const std::initializer_list<Argument<T>> args) {
        for (auto arg : args)
            init(arg);
    }

    /**
     * @brief Add an argument to the argument container
     */
    template <typename T> constexpr Argument<T> add(Argument<T> arg) {
        init(arg);
        return arg;
    }

    /**
     * @brief Check if the program was called with any unexpected arguments
     */
    constexpr void checkForUnexpected() const {
        for (size_t i{1}; i < rawArguments.size(); i++) {
            auto arg{rawArguments[i]};
            if (std::find(argumentNames.begin(), argumentNames.end(), arg) == argumentNames.end())
                throw exception("Unexpected argument '{}'", arg);
        }
    }

    /**
     * @brief Get the brief and full description of all arguments
     */
    constexpr std::tuple<std::string_view, std::string_view, std::string_view> getUsage() const {
        return std::make_tuple(programName.data(), briefUsageDescription.data(), usageDescription.data());
    }
};

} // namespace CommandLineArguments
