#include "util.h"
#include <fmt/core.h>
#include <set>
#include <string>
#include <vector>

namespace savepatcher::arguments {

struct ArgumentBase {
    constexpr virtual ~ArgumentBase() = default;
    constexpr ArgumentBase() = default;
    constexpr ArgumentBase(const ArgumentBase &) = default;
};

/**
 * @brief The container type for command line arguments
 */
template <typename T> struct Argument : ArgumentBase {
    const std::string_view name{};
    const std::string_view description{};
    const std::string_view briefDescription{};
    bool isSet{false};
    T value{};

    constexpr Argument(std::string_view name) : name(name) {}
    constexpr Argument(std::string_view name, std::string_view description) : name(name), description(description) {}
    constexpr Argument(std::string_view name, std::string_view briefDescription, std::string_view description) : name(name), description(description), briefDescription{briefDescription} {}
};

/**
 * @brief A command line argument parser
 */
class ArgumentParser {
  private:
    std::vector<std::shared_ptr<ArgumentBase>> argumentContainer; //!< The container with all static arguments
    const std::vector<std::string_view> rawArguments;             //!< The arguments passed to the program
    std::string briefUsageDescription;                            //!< A one line description of the required arguments
    std::string usageDescription;                                 //!< A full description of all arguments
    const std::string_view programName;                           //!< argv[0]

    /**
     * @brief Check if an argument is set
     */
    constexpr bool isArgued(std::string_view argumentName) const {
        return std::find(rawArguments.begin(), rawArguments.end(), argumentName) == rawArguments.end() ? false : true;
    }

    /**
     * @brief Get the value of the argument after the given name
     */
    constexpr std::string_view getNextArgument(std::string_view argumentName) const {
        const auto nextArgPos{static_cast<size_t>(std::distance(rawArguments.begin(), std::find(rawArguments.begin(), rawArguments.end(), argumentName)) + 1)};
        if (nextArgPos >= rawArguments.size())
            throw exception("Missing argument value for '{}'", argumentName);

        return rawArguments[nextArgPos];
    }

    constexpr void parse(Argument<bool> &arg) const {
        arg.value = true;
    }

    void parse(Argument<size_t> &arg) const {
        try {
            arg.value = std::stoi(getNextArgument(arg.name).data());
        } catch (const std::exception) {
            throw exception("Invalid value for '{}'", arg.name);
        }
    }

    constexpr void parse(Argument<std::string_view> &arg) const {
        arg.value = getNextArgument(arg.name);
    }

  public:
    ArgumentParser(int argc, char **argv) : rawArguments{argv, argv + argc}, programName{argv[0]} {}

    /**
     * @brief Parse an argument and append it to the argument container, if it is present
     */
    template <typename T> constexpr void add(const std::initializer_list<Argument<T>> params) {
        for (auto arg : params) {
            if (!arg.description.empty())
                usageDescription += fmt::format("  {}: {}\n", arg.name, arg.description);
            if (!arg.briefDescription.empty())
                briefUsageDescription += fmt::format("{} {} ", arg.name, arg.briefDescription);

            if (isArgued(arg.name)) {
                arg.isSet = true;
                parse(arg);
            }

            argumentContainer.emplace_back(std::make_shared<Argument<T>>(arg));
        }
    }

    /**
     * @brief Get the value of an argument
     */
    template <typename T> constexpr T get(std::string_view argumentName) const {
        if (isArgued(argumentName)) {
            for (auto &argptr : argumentContainer) {
                const auto arg{std::static_pointer_cast<Argument<T>>(argptr)};
                if (arg->name == argumentName && arg->isSet)
                    return arg->value;
            }
        }

        return T{};
    }

    /**
     * @brief Print a description of the arguments
     */
    void printUsage() const {
        fmt::print("Usage: {} {}\n{}", programName, briefUsageDescription, usageDescription);
    }
};

} // namespace savepatcher::ArgumentParser
