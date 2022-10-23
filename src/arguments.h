#include "util.h"
#include <concepts>
#include <vector>
#include <fmt/color.h>
#include <fmt/core.h>

namespace CommandLineArguments {

struct ArgumentBase {
  public:
    bool empty{true};
    std::string_view name{};
    std::string_view description{};
    std::string_view briefDescription{};

    constexpr virtual ~ArgumentBase() = default;
    constexpr ArgumentBase() = default;
    constexpr ArgumentBase(const ArgumentBase &) = default;
};

/**
 * @brief The container type for static command line arguments
 */
template <typename Type> struct Argument : public ArgumentBase {
    Type value{};
    bool set{};

    constexpr Argument() = default;
    constexpr Argument(std::string_view name, std::string_view description) {
        this->name = name;
        this->description = description;
    }

    constexpr Argument(std::string_view name, std::string_view briefDescription, std::string_view description) {
        this->name = name;
        this->briefDescription = briefDescription;
        this->description = description;
    }
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
    template <typename Type> constexpr void addArgument(Argument<Type> &arg) {
        arg.empty = false;
        if (isSet(arg.name)) {
            parse(arg);
            arg.set = true;
        }

        // TODO: dont assume formatting like this
        if (!arg.briefDescription.empty())
            briefUsageDescription += fmt::format("{} {} ", arg.name, arg.briefDescription);
        if (!arg.description.empty()) {
            if (!arg.briefDescription.empty())
                usageDescription += fmt::format("    {} {}: {}\n", arg.name, arg.briefDescription, arg.description);
            else
                usageDescription += fmt::format("    {}: {}\n", arg.name, arg.description);
        }

        argumentNames.emplace_back(arg.name);
        arguments.emplace_back(std::make_shared<Argument<Type>>(arg));
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

    /**
     * @brief Convert a string to a numeric type
     */
    template <typename Type> constexpr Type toNumber(std::string_view value, std::string_view argumentName = "") const {
        try {
            if constexpr (sizeof(u64) >= sizeof(Type))
                return static_cast<Type>(std::stoull(value.data()));
            else
                return static_cast<Type>(std::stoi(value.data()));
        } catch (const std::exception) {
            if (argumentName.empty())
                throw exception("Invalid argument value '{}'", value);
            else
                throw exception("Invalid argument value '{}' for '{}'", value, argumentName);
        }
    }

    /**
     * @brief Convert a string to a number if the type is arithmetic
     */
    template <typename Type> constexpr Type maybeNumber(std::string_view value) const {
        if constexpr (std::is_arithmetic_v<Type>)
            return toNumber<Type>(value);
        else
            return value;
    }

    template <typename First, typename Second> constexpr void parse(Argument<std::pair<First, Second>> &arg) {
        auto nextArgument{getNextArgument(arg.name)};
        arg.value = std::make_pair(maybeNumber<First>(nextArgument), maybeNumber<Second>(getNextArgument(nextArgument)));
    }

    template <typename Type>
    requires(!std::is_same<bool, Type>::value) constexpr void parse(Argument<Type> &arg) {
        arg.value = maybeNumber<Type>(getNextArgument(arg.name));
    }

    constexpr void parse(Argument<bool> &arg) const {
        arg.value = true;
    }

  public:
    ArgumentParser(int argc, char **argv) : rawArguments{argv, argv + argc}, programName{argv[0]} {}

    template <typename T> constexpr void add(const std::initializer_list<Argument<T>> args) {
        for (auto arg : args)
            addArgument(arg);
    }

    /**
     * @brief Add an argument to the argument container
     */
    template <typename T> constexpr Argument<T> add(Argument<T> arg) {
        addArgument(arg);
        return arg;
    }

    /**
     * @brief Check if the program was called with any unexpected arguments
     */
    constexpr void check() const {
        for (size_t i{1}; i < rawArguments.size(); i++) {
            auto arg{rawArguments[i]};
            if (std::find(argumentNames.begin(), argumentNames.end(), arg) == argumentNames.end())
                throw exception("Unexpected argument '{}'", arg);
        }
    }

    constexpr size_t size() const {
        return rawArguments.size() - 1;
    }

    void showUsage() {
        fmt::print("usage: {} {}\n{}", programName.data(), briefUsageDescription.data(), usageDescription.data());
    }
};

} // namespace CommandLineArguments
