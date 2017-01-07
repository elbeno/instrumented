#pragma once

#include <array>
#include <ostream>
#include <type_traits>
#include <utility>

struct instrumented_base
{
  enum Operation : int
  {
    DefaultConstruct,
    Construct,
    CopyConstruct,
    MoveConstruct,
    CopyAssign,
    MoveAssign,
    Destruct,
    Equality,
    Comparison,
    MaxOps
  };

  static auto get_op_name(int op)
  {
    static const char* s_name[MaxOps] = {
      "default construct",
      "construct",
      "copy construct",
      "move construct",
      "copy assign",
      "move assign",
      "destruct",
      "equality",
      "comparison"
    };
    return s_name[op];
  };
};

template <typename T>
struct instrumented : public instrumented_base
{
  template <typename = std::enable_if_t<std::is_default_constructible<T>::value>>
  instrumented() noexcept(std::is_nothrow_default_constructible<T>::value) {
    ++get_op_counts()[DefaultConstruct];
  }

  explicit instrumented(const T& a)
    noexcept(std::is_nothrow_copy_constructible<T>::value)
    : t(a) {
    ++get_op_counts()[Construct];
  }
  explicit instrumented(T&& a)
    noexcept(std::is_nothrow_move_constructible<T>::value)
    : t(std::move(a)) {
    ++get_op_counts()[Construct];
  }

  instrumented(const instrumented& a)
     noexcept(std::is_nothrow_copy_constructible<T>::value)
    : t(a.t) {
    ++get_op_counts()[CopyConstruct];
  }
  instrumented(instrumented&& a)
    noexcept(std::is_nothrow_move_constructible<T>::value)
    : t(std::move(a.t)) {
    ++get_op_counts()[MoveConstruct];
  }

  instrumented& operator=(const instrumented& a)
    noexcept(std::is_nothrow_copy_assignable<T>::value) {
    ++get_op_counts()[CopyAssign];
    t = a.t;
    return *this;
  }
  instrumented& operator=(instrumented&& a)
    noexcept(std::is_nothrow_move_assignable<T>::value) {
    ++get_op_counts()[MoveAssign];
    t = std::move(a.t);
    return *this;
  }

  ~instrumented() {
    ++get_op_counts()[Destruct];
  }

  T& value() { return t; }
  const T& value() const { return t; }

  static void reset_op_counts()
  {
    auto& counts = get_op_counts();
    counts.fill(0);
  }

  static std::ostream& output_op_names(std::ostream& s)
  {
    s << get_op_name(0);
    for (int i = 1; i < static_cast<int>(MaxOps); ++i)
    {
      s << ',' << get_op_name(i);
    }
    return s << '\n';
  }

  static std::ostream& output_op_counts(std::ostream& s)
  {
    const auto& counts = get_op_counts();
    s << counts[0];
    for (int i = 1; i < static_cast<int>(MaxOps); ++i)
    {
      s << ',' << counts[i];
    }
    return s << '\n';
  }

  friend bool operator==(const instrumented& a, const instrumented& b)
  {
    ++get_op_counts()[Equality];
    return a.t == b.t;
  }

  friend bool operator<(const instrumented& a, const instrumented& b)
  {
    ++get_op_counts()[Comparison];
    return a.t < b.t;
  }

private:
  static auto& get_op_counts()
  {
    static std::array<int, MaxOps> s_counts;
    return s_counts;
  };

  T t;
};

template <typename T>
inline bool operator>(const instrumented<T>& a, const instrumented<T>& b)
{
  return b < a;
}

template <typename T>
inline bool operator<=(const instrumented<T>& a, const instrumented<T>& b)
{
  return !(b < a);
}

template <typename T>
inline bool operator>=(const instrumented<T>& a, const instrumented<T>& b)
{
  return !(a < b);
}
