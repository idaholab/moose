//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/utility.h"
#include "libmesh/compare_types.h"
#include "Conversion.h"
#include <limits>

namespace CompileTimeDerivatives
{

/**
 * All compile time derivative system objects derive from this (empty) base class.
 * This allows to restrict templated operators to only act on compile time derivative
 * system objects (using std::enable_if).
 */
class CTBase
{
public:
  /// precedence should reflect C++ operator precedence exactly (higher is binding tighter)
  constexpr static int precedence() { return 0; }
  /// left/right associative property should reflect C++ operator properties exactly
  constexpr static bool leftAssociative() { return false; }
};
class CTNullBase : public CTBase
{
};
class CTOneBase : public CTBase
{
};

template <typename T>
using CTCleanType = typename std::remove_const<typename std::remove_reference<T>::type>::type;

template <typename... Ts>
struct CTSuperType;

template <typename T>
struct CTSuperType<T>
{
  typedef T type;
};

template <typename T1, typename T2, typename... Ts>
struct CTSuperType<T1, T2, Ts...>
{
  typedef typename std::conditional<
      (sizeof...(Ts) > 0),
      typename CTSuperType<typename libMesh::CompareTypes<T1, T2>::supertype, Ts...>::type,
      typename libMesh::CompareTypes<T1, T2>::supertype>::type type;
};

/**
 * Operators representing variable values need to be tagged. The tag identifies
 * the object when a compile time derivative is taken. We use a type that can be
 * supplied as a template argument. int does the trick.
 */
using CTTag = int;
constexpr CTTag CTNoTag = std::numeric_limits<CTTag>::max();

template <CTTag tag>
std::string
printTag()
{
  if constexpr (tag == CTNoTag)
    return "";
  else
    return Moose::stringify(tag);
}

template <typename B, typename E>
auto pow(const B & base, const E & exp);
template <int E, typename B>
auto pow(const B &);

template <typename T>
auto exp(const T &);
template <typename T>
auto log(const T &);

/**
 * Template class to represent a "zero" value. Having zeroness associated with a type
 * enables compile time optimizations in some operators.
 */
template <typename T>
class CTNull : public CTNullBase
{
public:
  CTNull() {}
  typedef CTCleanType<T> ResultType;

  ResultType operator()() const { return ResultType(0); }
  std::string print() const { return "0"; }

  template <CTTag dtag>
  auto D() const
  {
    return CTNull<ResultType>();
  }
};

/**
 * Template class to represent a "one" value. Having oneness associated with a type
 * enables compile time optimizations in some operators.
 */
template <typename T>
class CTOne : public CTOneBase
{
public:
  CTOne() {}
  typedef CTCleanType<T> ResultType;

  ResultType operator()() const { return ResultType(1); }
  std::string print() const { return "1"; }

  template <CTTag dtag>
  auto D() const
  {
    return CTNull<ResultType>();
  }
};

/**
 * Base class for a unary operator/function
 */
template <typename T>
class CTUnary : public CTBase
{
public:
  CTUnary(T arg) : _arg(arg) {}

  template <typename Self>
  std::string printParens(const Self *, const std::string & op) const
  {
    std::string out = op;
    if constexpr (T::precedence() > Self::precedence())
      out += "(" + _arg.print() + ")";
    else
      out += _arg.print();

    return out;
  }

  typedef typename T::ResultType ResultType;

protected:
  const T _arg;
};

/**
 * Unary minus
 */
template <typename T>
class CTUnaryMinus : public CTUnary<T>
{
public:
  CTUnaryMinus(T arg) : CTUnary<T>(arg) {}
  using typename CTUnary<T>::ResultType;

  ResultType operator()() const { return -_arg(); }
  std::string print() const { return this->printParens(this, "-"); }
  constexpr static int precedence() { return 3; }

  template <CTTag dtag>
  auto D() const
  {
    return -_arg.template D<dtag>();
  }

  using CTUnary<T>::_arg;
};

template <typename T, class = std::enable_if_t<std::is_base_of<CTBase, T>::value>>
auto
operator-(const T & arg)
{
  return CTUnaryMinus<T>(arg);
}

/**
 * Base class for a binary operator/function
 */
template <typename L, typename R>
class CTBinary : public CTBase
{
public:
  CTBinary(L left, R right) : _left(left), _right(right) {}

  typedef typename libMesh::CompareTypes<typename L::ResultType, typename R::ResultType>::supertype
      ResultType;

  template <typename Self>
  std::string printParens(const Self *, const std::string & op) const
  {
    std::string out;
    if constexpr (L::precedence() > Self::precedence())
      out = "(" + _left.print() + ")";
    else
      out = _left.print();

    out += op;

    if (R::precedence() > Self::precedence() ||
        (R::precedence() == Self::precedence() && Self::leftAssociative()))
      out += "(" + _right.print() + ")";
    else
      out += _right.print();

    return out;
  }

protected:
  const L _left;
  const R _right;
};

template <typename C, typename L, typename R>
auto conditional(const C &, const L &, const R &);

/**
 * Base class for a ternary functions
 */
template <typename C, typename L, typename R>
class CTConditional : public CTBinary<L, R>
{
public:
  CTConditional(C condition, L left, R right) : CTBinary<L, R>(left, right), _condition(condition)
  {
  }
  using typename CTBinary<L, R>::ResultType;

  auto operator()() const { return _condition() ? _left() : _right(); }
  template <CTTag dtag>
  auto D() const
  {
    return conditional(_condition, _left.template D<dtag>(), _right.template D<dtag>());
  }

  template <typename Self>
  std::string print() const
  {
    return "conditional(" + _condition.print() + ", " + _left.print() + ", " + _right.print() + ")";
  }

protected:
  const C _condition;

  using CTBinary<L, R>::_left;
  using CTBinary<L, R>::_right;
};

template <typename C, typename L, typename R>
auto
conditional(const C & condition, const L & left, const R & right)
{
  return CTConditional<C, L, R>(condition, left, right);
}

template <typename L, typename R>
auto
min(const L & left, const R & right)
{
  return CTConditional<decltype(left < right), L, R>(left < right, left, right);
}

template <typename L, typename R>
auto
max(const L & left, const R & right)
{
  return CTConditional<decltype(left > right), L, R>(left > right, left, right);
}

/**
 * Constant value
 */
template <CTTag tag, typename T>
class CTValue : public CTBase
{
public:
  CTValue(const T value) : _value(value) {}
  typedef T ResultType;

  auto operator()() const { return _value; }
  template <CTTag dtag>
  auto D() const
  {
    return CTNull<ResultType>();
  }

  std::string print() const { return Moose::stringify(_value); }

protected:
  T _value;
};

/**
 * Helper function to build a (potentially tagged) value
 */
template <CTTag tag = CTNoTag, typename T>
auto
makeValue(T value)
{
  return CTValue<tag, T>(value);
}

template <CTTag start_tag, typename... Values, CTTag... Tags>
auto
makeValuesHelper(const std::tuple<Values...> & values, std::integer_sequence<CTTag, Tags...>)
{
  if constexpr (start_tag == CTNoTag)
    return std::make_tuple(CTValue<CTNoTag, Values>(std::get<Tags>(values))...);
  else
    return std::make_tuple(CTValue<Tags + start_tag, Values>(std::get<Tags>(values))...);
}

/**
 * Helper function to build a list of (potentially tagged) values
 */
template <CTTag start_tag = CTNoTag, typename... Ts>
auto
makeValues(Ts... values)
{
  return makeValuesHelper<start_tag>(std::tuple(values...),
                                     std::make_integer_sequence<CTTag, sizeof...(values)>{});
}

/**
 * Variable value, referencing a variable of type T. This object is tagged with a
 * CTTag to enable taking symbolic derivatives.
 */
template <CTTag tag, typename T>
class CTRef : public CTBase
{
public:
  CTRef(const T & ref) : _ref(ref) {}
  const T & operator()() const { return _ref; }
  std::string print() const { return "[v" + printTag<tag>() + "]"; }

  template <CTTag dtag>
  auto D() const
  {
    if constexpr (tag == dtag)
      return CTOne<ResultType>();
    else
      return CTNull<ResultType>();
  }

  typedef CTCleanType<T> ResultType;

protected:
  const T & _ref;
};

/**
 * Helper function to build a tagged reference to a variable
 */
template <CTTag tag = CTNoTag, typename T>
auto
makeRef(const T & ref)
{
  return CTRef<tag, T>(ref);
}

template <CTTag start_tag, typename... Refs, CTTag... Tags>
auto
makeRefsHelper(const std::tuple<Refs...> & refs, std::integer_sequence<CTTag, Tags...>)
{
  if constexpr (start_tag == CTNoTag)
    return std::make_tuple(CTRef<CTNoTag, Refs>(std::get<Tags>(refs))...);
  else
    return std::make_tuple(CTRef<Tags + start_tag, Refs>(std::get<Tags>(refs))...);
}

/**
 * Helper function to build a list of tagged references to variables
 */
template <CTTag start_tag = CTNoTag, typename... Ts>
auto
makeRefs(const Ts &... refs)
{
  return makeRefsHelper<start_tag>(std::tie(refs...),
                                   std::make_integer_sequence<CTTag, sizeof...(refs)>{});
}

/**
 * Array variable value, referencing an entry in an indexable container of T types.
 * The index of type I is also stored as a reference.
 * This object is tagged with a CTTag to enable taking symbolic derivatives.
 */
template <CTTag tag, typename T, typename I>
class CTArrayRef : public CTBase
{
public:
  CTArrayRef(const T & arr, const I & idx) : _arr(arr), _idx(idx) {}
  auto operator()() const { return _arr[_idx]; }
  std::string print() const { return "[a" + printTag<tag>() + "[" + Moose::stringify(_idx) + "]]"; }

  template <CTTag dtag>
  auto D() const
  {
    if constexpr (tag == dtag)
      return CTOne<ResultType>();
    else
      return CTNull<ResultType>();
  }

  // get the value type returned by operator[]
  typedef CTCleanType<decltype((static_cast<T>(0))[0])> ResultType;
  static_assert(!std::is_same_v<ResultType, void>,
                "Instantiation of CTArrayRef was attempted for a non-subscriptable type.");

protected:
  const T & _arr;
  const I & _idx;
};

/**
 * Helper function to build a tagged reference to a vector/array entry
 */
template <CTTag tag = CTNoTag, typename T, typename I>
auto
makeRef(const T & ref, const I & idx)
{
  return CTArrayRef<tag, T, I>(ref, idx);
}

/**
 * Addition operator node
 */
template <typename L, typename R>
class CTAdd : public CTBinary<L, R>
{
public:
  CTAdd(L left, R right) : CTBinary<L, R>(left, right) {}
  using typename CTBinary<L, R>::ResultType;

  ResultType operator()() const
  {
    // compile time optimization to skip null terms
    if constexpr (std::is_base_of<CTNullBase, L>::value && std::is_base_of<CTNullBase, R>::value)
      return ResultType(0);

    if constexpr (std::is_base_of<CTNullBase, L>::value)
      return _right();

    if constexpr (std::is_base_of<CTNullBase, R>::value)
      return _left();

    else
      return _left() + _right();
  }
  std::string print() const { return this->printParens(this, "+"); }
  constexpr static int precedence() { return 6; }

  template <CTTag dtag>
  auto D() const
  {
    return _left.template D<dtag>() + _right.template D<dtag>();
  }

  using CTBinary<L, R>::_left;
  using CTBinary<L, R>::_right;
};

/**
 * Subtraction operator node
 */
template <typename L, typename R>
class CTSub : public CTBinary<L, R>
{
public:
  CTSub(L left, R right) : CTBinary<L, R>(left, right) {}
  using typename CTBinary<L, R>::ResultType;

  ResultType operator()() const
  {
    if constexpr (std::is_base_of<CTNullBase, L>::value && std::is_base_of<CTNullBase, R>::value)
      return ResultType(0);

    if constexpr (std::is_base_of<CTNullBase, L>::value)
      return -_right();

    if constexpr (std::is_base_of<CTNullBase, R>::value)
      return _left();

    else
      return _left() - _right();
  }
  std::string print() const { return this->printParens(this, "-"); }
  constexpr static int precedence() { return 6; }
  constexpr static bool leftAssociative() { return true; }

  template <CTTag dtag>
  auto D() const
  {
    return _left.template D<dtag>() - _right.template D<dtag>();
  }

  using CTBinary<L, R>::_left;
  using CTBinary<L, R>::_right;
};

/**
 * Multiplication operator node
 */
template <typename L, typename R>
class CTMul : public CTBinary<L, R>
{
public:
  CTMul(L left, R right) : CTBinary<L, R>(left, right) {}
  using typename CTBinary<L, R>::ResultType;

  ResultType operator()() const
  {
    if constexpr (std::is_base_of<CTNullBase, L>::value || std::is_base_of<CTNullBase, R>::value)
      return ResultType(0);

    if constexpr (std::is_base_of<CTOneBase, L>::value && std::is_base_of<CTOneBase, R>::value)
      return ResultType(1);

    if constexpr (std::is_base_of<CTOneBase, L>::value)
      return _right();

    if constexpr (std::is_base_of<CTOneBase, R>::value)
      return _left();

    else
      return _left() * _right();
  }
  std::string print() const { return this->printParens(this, "*"); }
  constexpr static int precedence() { return 5; }

  template <CTTag dtag>
  auto D() const
  {
    return _left.template D<dtag>() * _right + _right.template D<dtag>() * _left;
  }

  using CTBinary<L, R>::_left;
  using CTBinary<L, R>::_right;
};

/**
 * Division operator node
 */
template <typename L, typename R>
class CTDiv : public CTBinary<L, R>
{
public:
  CTDiv(L left, R right) : CTBinary<L, R>(left, right) {}
  using typename CTBinary<L, R>::ResultType;

  ResultType operator()() const
  {
    if constexpr (std::is_base_of<CTOneBase, R>::value)
      return _left();

    if constexpr (std::is_base_of<CTNullBase, L>::value && !std::is_base_of<CTNullBase, R>::value)
      return ResultType(0);

    return _left() / _right();
  }
  std::string print() const { return this->printParens(this, "/"); }
  constexpr static int precedence() { return 5; }
  constexpr static bool leftAssociative() { return true; }

  template <CTTag dtag>
  auto D() const
  {
    return _left.template D<dtag>() / _right -
           _left * _right.template D<dtag>() / (_right * _right);
  }

  using CTBinary<L, R>::_left;
  using CTBinary<L, R>::_right;
};

enum class CTComparisonEnum
{
  Less,
  LessEqual,
  Greater,
  GreaterEqual,
  Equal,
  Unequal
};

/**
 * Binary comparison operator node
 */
template <CTComparisonEnum C, typename L, typename R>
class CTCompare : public CTBinary<L, R>
{
public:
  CTCompare(L left, R right) : CTBinary<L, R>(left, right) {}
  typedef bool ResultType;

  ResultType operator()() const
  {
    if constexpr (C == CTComparisonEnum::Less)
      return _left() < _right();
    if constexpr (C == CTComparisonEnum::LessEqual)
      return _left() <= _right();
    if constexpr (C == CTComparisonEnum::Greater)
      return _left() > _right();
    if constexpr (C == CTComparisonEnum::GreaterEqual)
      return _left() >= _right();
    if constexpr (C == CTComparisonEnum::Equal)
      return _left() == _right();
    if constexpr (C == CTComparisonEnum::Unequal)
      return _left() != _right();
  }
  std::string print() const
  {
    if constexpr (C == CTComparisonEnum::Less)
      return this->printParens(this, "<");
    if constexpr (C == CTComparisonEnum::LessEqual)
      return this->printParens(this, "<=");
    if constexpr (C == CTComparisonEnum::Greater)
      return this->printParens(this, ">");
    if constexpr (C == CTComparisonEnum::GreaterEqual)
      return this->printParens(this, ">=");
    if constexpr (C == CTComparisonEnum::Equal)
      return this->printParens(this, "==");
    if constexpr (C == CTComparisonEnum::Unequal)
      return this->printParens(this, "!=");
  }
  constexpr static int precedence() { return 9; }
  constexpr static bool leftAssociative() { return true; }

  template <CTTag dtag>
  auto D() const
  {
    return CTNull<ResultType>();
  }

  using CTBinary<L, R>::_left;
  using CTBinary<L, R>::_right;
};

/// template aliases for the comparison operator nodes
template <typename L, typename R>
using CTCompareLess = CTCompare<CTComparisonEnum::Less, L, R>;
template <typename L, typename R>
using CTCompareLessEqual = CTCompare<CTComparisonEnum::LessEqual, L, R>;
template <typename L, typename R>
using CTCompareGreater = CTCompare<CTComparisonEnum::Greater, L, R>;
template <typename L, typename R>
using CTCompareGreaterEqual = CTCompare<CTComparisonEnum::GreaterEqual, L, R>;
template <typename L, typename R>
using CTCompareEqual = CTCompare<CTComparisonEnum::Equal, L, R>;
template <typename L, typename R>
using CTCompareUnequal = CTCompare<CTComparisonEnum::Unequal, L, R>;

/**
 * Power operator where both base and exponent can be arbitrary operators.
 */
template <typename L, typename R>
class CTPow : public CTBinary<L, R>
{
public:
  CTPow(L left, R right) : CTBinary<L, R>(left, right) {}
  using typename CTBinary<L, R>::ResultType;

  ResultType operator()() const
  {
    if constexpr (std::is_base_of<CTNullBase, L>::value)
      return ResultType(0);

    if constexpr (std::is_base_of<CTOneBase, L>::value || std::is_base_of<CTNullBase, R>::value)
      return ResultType(1);

    if constexpr (std::is_base_of<CTOneBase, R>::value)
      return _left();

    return std::pow(_left(), _right());
  }
  std::string print() const { return "pow(" + _left.print() + "," + _right.print() + ")"; }

  template <CTTag dtag>
  auto D() const
  {
    if constexpr (std::is_base_of<CTNullBase, decltype(_left.template D<dtag>())>::value &&
                  std::is_base_of<CTNullBase, decltype(_right.template D<dtag>())>::value)
      return CTNull<ResultType>();

    else if constexpr (std::is_base_of<CTNullBase, decltype(_left.template D<dtag>())>::value)
      return pow(_left, _right) * _right.template D<dtag>() * log(_left);

    else if constexpr (std::is_base_of<CTNullBase, decltype(_right.template D<dtag>())>::value)
      return pow(_left, _right) * _right * _left.template D<dtag>() / _left;

    else
      return pow(_left, _right) *
             (_right.template D<dtag>() * log(_left) + _right * _left.template D<dtag>() / _left);
  }

  using CTBinary<L, R>::_left;
  using CTBinary<L, R>::_right;
};

/**
 * pow(base, exponent) function overload.
 */
template <typename B, typename E>
auto
pow(const B & base, const E & exp)
{
  if constexpr (std::is_base_of<CTBase, B>::value && std::is_base_of<CTBase, E>::value)
    return CTPow(base, exp);
  else if constexpr (std::is_base_of<CTBase, E>::value)
    return CTPow(makeValue(base), exp);
  else if constexpr (std::is_base_of<CTBase, B>::value)
    return CTPow(base, makeValue(exp));
  else
    return CTPow(makeValue(base), makeValue(exp));
}

/**
 * Integer exponent power operator.
 */
template <typename B, int E>
class CTIPow : public CTUnary<B>
{
public:
  CTIPow(B base) : CTUnary<B>(base) {}
  using typename CTUnary<B>::ResultType;

  ResultType operator()() const
  {
    if constexpr (std::is_base_of<CTNullBase, B>::value)
      return ResultType(0);

    else if constexpr (std::is_base_of<CTOneBase, B>::value || E == 0)
      return ResultType(1);

    else if constexpr (E == 1)
      return _arg();

    else if constexpr (E < 0)
      return 1.0 / libMesh::Utility::pow<-E>(_arg());

    else
      return libMesh::Utility::pow<E>(_arg());
  }
  std::string print() const { return "pow<" + Moose::stringify(E) + ">(" + _arg.print() + ")"; }

  template <CTTag dtag>
  auto D() const
  {
    if constexpr (E == 1)
      return _arg.template D<dtag>();

    else if constexpr (E == 0)
      return CTNull<ResultType>();

    else
      return pow<E - 1>(_arg) * E * _arg.template D<dtag>();
  }

  using CTUnary<B>::_arg;
};

/**
 * pow<exponent>(base) template for integer powers.
 */
template <int E, typename B>
auto
pow(const B & base)
{
  if constexpr (std::is_base_of<CTBase, B>::value)
    return CTIPow<B, E>(base);
  else
    return CTIPow<CTValue<CTNoTag, B>, E>(makeValue(base));
}

/**
 * Macro for implementing a binary math operator overload that works with a mix of CT system
 * objects, C variables, and number literals.
 */
#define CT_OPERATOR_BINARY(op, OP)                                                                 \
  template <typename L,                                                                            \
            typename R,                                                                            \
            class = std::enable_if_t<std::is_base_of<CTBase, L>::value ||                          \
                                     std::is_base_of<CTBase, R>::value>>                           \
  auto operator op(const L & left, const R & right)                                                \
  {                                                                                                \
    /* We need a template arguments here because:      */                                          \
    /* alias template deduction is only available with '-std=c++2a' or '-std=gnu++2a'      */      \
    if constexpr (std::is_base_of<CTBase, L>::value && std::is_base_of<CTBase, R>::value)          \
      return OP<L, R>(left, right);                                                                \
    else if constexpr (std::is_base_of<CTBase, L>::value)                                          \
      return OP<L, decltype(makeValue(right))>(left, makeValue(right));                            \
    else if constexpr (std::is_base_of<CTBase, R>::value)                                          \
      return OP<decltype(makeValue(left)), R>(makeValue(left), right);                             \
    else                                                                                           \
      static_assert(libMesh::always_false<L>, "This should not be instantiated.");                 \
  }

CT_OPERATOR_BINARY(+, CTAdd)
CT_OPERATOR_BINARY(-, CTSub)
CT_OPERATOR_BINARY(*, CTMul)
CT_OPERATOR_BINARY(/, CTDiv)
CT_OPERATOR_BINARY(<, CTCompareLess)
CT_OPERATOR_BINARY(<=, CTCompareLessEqual)
CT_OPERATOR_BINARY(>, CTCompareGreater)
CT_OPERATOR_BINARY(>=, CTCompareGreaterEqual)
CT_OPERATOR_BINARY(==, CTCompareEqual)
CT_OPERATOR_BINARY(!=, CTCompareUnequal)

/**
 * Macro for implementing a simple unary function overload. No function specific optimizations are
 * possible. The parameters are the function name and the expression that returns the derivative
 * of the function.
 */
#define CT_SIMPLE_UNARY_FUNCTION(name, derivative)                                                 \
  template <typename T>                                                                            \
  class CTF##name : public CTUnary<T>                                                              \
  {                                                                                                \
  public:                                                                                          \
    CTF##name(T arg) : CTUnary<T>(arg) {}                                                          \
    auto operator()() const { return std::name(_arg()); }                                          \
    template <CTTag dtag>                                                                          \
    auto D() const                                                                                 \
    {                                                                                              \
      return derivative;                                                                           \
    }                                                                                              \
    std::string print() const { return #name "(" + _arg.print() + ")"; }                           \
    constexpr static int precedence() { return 2; }                                                \
    using typename CTUnary<T>::ResultType;                                                         \
    using CTUnary<T>::_arg;                                                                        \
  };                                                                                               \
  template <typename T>                                                                            \
  auto name(const T & v)                                                                           \
  {                                                                                                \
    using namespace CompileTimeDerivatives;                                                        \
    if constexpr (std::is_base_of<CTBase, T>::value)                                               \
      return CTF##name(v);                                                                         \
    else                                                                                           \
      return CTF##name(makeValue(v));                                                              \
  }

CT_SIMPLE_UNARY_FUNCTION(exp, exp(_arg) * _arg.template D<dtag>())
CT_SIMPLE_UNARY_FUNCTION(log, _arg.template D<dtag>() / _arg)
CT_SIMPLE_UNARY_FUNCTION(sin, cos(_arg) * _arg.template D<dtag>())
CT_SIMPLE_UNARY_FUNCTION(cos, -1.0 * sin(_arg) * _arg.template D<dtag>())
CT_SIMPLE_UNARY_FUNCTION(tan, (pow<2>(tan(_arg)) + 1.0) * _arg.template D<dtag>())
CT_SIMPLE_UNARY_FUNCTION(sqrt, 1.0 / (2.0 * sqrt(_arg)) * _arg.template D<dtag>())
CT_SIMPLE_UNARY_FUNCTION(tanh, (1.0 - pow<2>(tanh(_arg))) * _arg.template D<dtag>())
CT_SIMPLE_UNARY_FUNCTION(sinh, cosh(_arg) * _arg.template D<dtag>())
CT_SIMPLE_UNARY_FUNCTION(cosh, sinh(_arg) * _arg.template D<dtag>())
CT_SIMPLE_UNARY_FUNCTION(erf,
                         2.0 * exp(-pow<2>(_arg)) / sqrt(libMesh::pi) * _arg.template D<dtag>())
CT_SIMPLE_UNARY_FUNCTION(atan, 1.0 / (pow<2>(_arg) + 1.0) * _arg.template D<dtag>())

/**
 * Macro for implementing a simple binary function overload. No function specific optimizations are
 * possible. The parameters are the function name and the expression that returns the derivative
 * of the function.
 */
#define CT_SIMPLE_BINARY_FUNCTION_CLASS(name, derivative)                                          \
  template <typename L, typename R>                                                                \
  class CTF##name : public CTBinary<L, R>                                                          \
  {                                                                                                \
  public:                                                                                          \
    CTF##name(L left, R right) : CTBinary<L, R>(left, right) {}                                    \
    auto operator()() const { return std::name(_left(), _right()); }                               \
    template <CTTag dtag>                                                                          \
    auto D() const                                                                                 \
    {                                                                                              \
      return derivative;                                                                           \
    }                                                                                              \
    std::string print() const { return #name "(" + _left.print() + ", " + _right.print() + ")"; }  \
    constexpr static int precedence() { return 2; }                                                \
    using typename CTBinary<L, R>::ResultType;                                                     \
    using CTBinary<L, R>::_left;                                                                   \
    using CTBinary<L, R>::_right;                                                                  \
  };
#define CT_SIMPLE_BINARY_FUNCTION_FUNC(name)                                                       \
  template <typename L, typename R>                                                                \
  auto name(const L & l, const R & r)                                                              \
  {                                                                                                \
    using namespace CompileTimeDerivatives;                                                        \
    if constexpr (std::is_base_of<CTBase, L>::value && std::is_base_of<CTBase, R>::value)          \
      return CTF##name(l, r);                                                                      \
    else if constexpr (std::is_base_of<CTBase, L>::value)                                          \
      return CTF##name(l, makeValue(r));                                                           \
    else if constexpr (std::is_base_of<CTBase, R>::value)                                          \
      return CTF##name(makeValue(l), r);                                                           \
    else                                                                                           \
      return CTF##name(makeValue(l), makeValue(r));                                                \
  }

CT_SIMPLE_BINARY_FUNCTION_CLASS(atan2,
                                (-_left * _right.template D<dtag>() +
                                 _left.template D<dtag>() * _right) /
                                    (pow<2>(_left) + pow<2>(_right)))
CT_SIMPLE_BINARY_FUNCTION_FUNC(atan2)

template <typename T, int N, int M>
class CTMatrix
{
public:
  template <typename... Ts>
  CTMatrix(Ts... a) : _data({a...})
  {
    static_assert(sizeof...(a) == N * M, "Invalid number of matrix entries");
  }
  T & operator()(std::size_t n, std::size_t m) { return _data[M * n + m]; }
  const T & operator()(std::size_t n, std::size_t m) const { return _data[M * n + m]; }

protected:
  std::array<T, N * M> _data;
};

template <typename... Ds>
class CTStandardDeviation : public CTBase
{
public:
  static constexpr auto N = sizeof...(Ds);

  CTStandardDeviation(std::tuple<Ds...> derivatives, CTMatrix<Real, N, N> covariance)
    : _derivatives(derivatives), _covariance(covariance)
  {
  }
  auto operator()() const { return std::sqrt(evalHelper(std::make_index_sequence<N>{})); }

  typedef typename CTSuperType<typename Ds::ResultType...>::type ResultType;

protected:
  template <int R, std::size_t... Is>
  ResultType rowMul(std::index_sequence<Is...>, const std::array<ResultType, N> & d) const
  {
    return ((_covariance(R, Is) * d[Is]) + ...);
  }

  template <std::size_t... Is>
  auto evalHelper(const std::index_sequence<Is...> & is) const
  {
    const std::array<ResultType, N> d{std::get<Is>(_derivatives)()...};
    return ((rowMul<Is>(is, d) * d[Is]) + ...);
  }

  const std::tuple<Ds...> _derivatives;
  const CTMatrix<Real, N, N> _covariance;
};

template <CTTag start_tag, typename T, CTTag... Tags>
auto
makeStandardDeviationHelper(const T & f, std::integer_sequence<CTTag, Tags...>)
{
  return std::make_tuple(f.template D<Tags + start_tag>()...);
}

/**
 * Helper function to build a standard deviation object for a function with N parameters with
 * consecutive tags starting at start_tag, and an NxN covariance matrix for said parameters.
 */
template <CTTag start_tag, typename T, int N>
auto
makeStandardDeviation(const T & f, const CTMatrix<Real, N, N> covariance)
{
  return CTStandardDeviation(
      makeStandardDeviationHelper<start_tag>(f, std::make_integer_sequence<CTTag, N>{}),
      covariance);
}

} // namespace CompileTimeDerivatives
