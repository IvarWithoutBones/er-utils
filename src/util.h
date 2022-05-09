#include <stdexcept>
#include <fmt/format.h>

namespace savepatcher {

// Copied from https://github.com/skyline-emu/skyline/blob/8826c113b0bdd602ba302df7ad47febe751d45d1/app/src/main/cpp/skyline/common/base.h#L76
// A wrapper around std::runtime_error with {fmt} formatting
class exception : public std::runtime_error {
  public:
    template<typename S, typename... Args>
    constexpr auto Format(S formatString, Args &&... args) {
        return fmt::format(fmt::runtime(formatString), FmtCast(args)...);
    }

    template<typename T>
    constexpr auto FmtCast(T object) {
        if constexpr (std::is_pointer<T>::value)
            if constexpr (std::is_same<char, typename std::remove_cv<typename std::remove_pointer<T>::type>::type>::value)
                return reinterpret_cast<typename std::common_type<char *, T>::type>(object);
            else
                return reinterpret_cast<const uintptr_t>(object);
        else
            return object;
    }

    template<typename S, typename... Args>
    exception(const S &formatStr, Args &&... args) : runtime_error(Format(formatStr, args...)) {}
};

} // namespace savepatcher::util
