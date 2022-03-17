#pragma once

#include <tuple>
#include <type_traits>

namespace modl {

namespace detail {

template <typename> struct module_function_traits_t;

template <typename R, typename... Args>
struct module_function_traits_t<R(Args...)> {
  static_assert(std::is_function_v<R(Args...)>, "Must be a function type");
  using type = R(Args...);
  using return_type = R;
  using pointer_type = std::add_pointer_t<type>;
  using args = std::tuple<Args...>;

  template <size_t i> struct arg {
    using type = typename std::tuple_element<i, args>::type;
  };

  static constexpr size_t arg_count = sizeof...(Args);
};

template <auto Val>
using module_function_traits =
    module_function_traits_t<std::remove_pointer_t<decltype(Val)>>;

} // namespace detail

template <typename Traits> struct module_function {
public:
  using traits = Traits;
  using type = typename traits::type;
  using pointer_type = typename traits::pointer_type;

  explicit module_function(pointer_type x) noexcept : value_(x) {}

  template <typename... Args> typename traits::return_type call(Args &&...x) {
    return value_(std::forward<Args>(x)...);
  }

  pointer_type get() const noexcept { return value_; }
  pointer_type operator()() const noexcept { return value_; }
  operator pointer_type() const noexcept { return value_; }

private:
  pointer_type value_;
};

} // namespace modl
