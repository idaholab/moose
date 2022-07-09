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

namespace CompileTimeDerivatives
{

/**
 * All compile time derivative system objects derive from this (empty) base class.
 * This allows to restrict templated operators to only act on compile time derivative
 * system objects (using std::enable_if).
 */
class CTBase
{
};
class CTNullBase : public CTBase
{
};
class CTOneBase : public CTBase
{
};

template <typename T1, typename T2>
struct CTSuperType
{
  typedef double type;
};

/**
 * Operators representing variable values need to be tagged. The tag identifies
 * the object when a compile time derivative is taken. We use a type that can be
 * supplied as a template argument. int does the trick.
 */
using CTTag = int;

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
  auto operator()() const { return O(0); }

  template <CTTag dtag>
  auto D() const
  {
    return CTNull<O>();
  }

  typedef T O;
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
  auto operator()() const { return O(1); }

  template <CTTag dtag>
  auto D() const
  {
    return CTNull<O>();
  }

  typedef T O;
};

/**
 * Base class for a unary operator/function
 */
template <typename T>
class CTUnary : public CTBase
{
public:
  CTUnary(T arg) : _arg(arg) {}

  typedef typename T::O O;

protected:
  const T _arg;
};

/**
 * Base class for a binary operator/function
 */
template <typename L, typename R>
class CTBinary : public CTBase
{
public:
  CTBinary(L left, R right) : _left(left), _right(right) {}

  typedef typename CTSuperType<typename L::O, typename R::O>::type O;

protected:
  const L _left;
  const R _right;
};

/**
 * Constant value
 */
template <typename T>
class CTValue : public CTBase
{
public:
  CTValue(const T value) : _value(value) {}
  auto operator()() const { return _value; }

  template <CTTag dtag>
  auto D() const
  {
    return CTNull<O>();
  }

  typedef T O;

protected:
  T _value;
};

/**
 * Variable value, referencing a variable of type T. This object is tagged with a
 * CTTag to enable taking symbolic derivatives.
 */
template <CTTag tag, typename T>
class CTRef : public CTBase
{
public:
  CTRef(const T & ref) : _ref(ref) {}
  auto operator()() const { return _ref; }

  template <CTTag dtag>
  auto D() const
  {
    if constexpr (tag == dtag)
      return CTOne<O>();
    else
      return CTNull<O>();
  }

  typedef T O;

protected:
  const T & _ref;
};

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

  template <CTTag dtag>
  auto D() const
  {
    if constexpr (tag == dtag)
      return CTOne<O>();
    else
      return CTNull<O>();
  }

  typedef typename T::value_type O;

protected:
  const T & _arr;
  const I & _idx;
};

/**
 * Addition operator node
 */
template <typename L, typename R>
class CTAdd : public CTBinary<L, R>
{
public:
  CTAdd(L left, R right) : CTBinary<L, R>(left, right) {}
  auto operator()() const
  {
    // compile time optimization to skip null terms
    if constexpr (std::is_base_of<CTNullBase, L>::value && std::is_base_of<CTNullBase, R>::value)
      return O(0);

    if constexpr (std::is_base_of<CTNullBase, L>::value)
      return _right();

    if constexpr (std::is_base_of<CTNullBase, R>::value)
      return _left();

    else
      return _left() + _right();
  }

  template <CTTag dtag>
  auto D() const
  {
    return _left.template D<dtag>() + _right.template D<dtag>();
  }

  using typename CTBinary<L, R>::O;
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
  auto operator()() const
  {
    if constexpr (std::is_base_of<CTNullBase, L>::value && std::is_base_of<CTNullBase, R>::value)
      return O(0);

    if constexpr (std::is_base_of<CTNullBase, L>::value)
      return -_right();

    if constexpr (std::is_base_of<CTNullBase, R>::value)
      return _left();

    else
      return _left() - _right();
  }

  template <CTTag dtag>
  auto D() const
  {
    return _left.template D<dtag>() - _right.template D<dtag>();
  }

  using typename CTBinary<L, R>::O;
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
  auto operator()() const
  {
    if constexpr (std::is_base_of<CTNullBase, L>::value || std::is_base_of<CTNullBase, R>::value)
      return O(0);

    if constexpr (std::is_base_of<CTOneBase, L>::value && std::is_base_of<CTOneBase, R>::value)
      return O(1);

    if constexpr (std::is_base_of<CTOneBase, L>::value)
      return _right();

    if constexpr (std::is_base_of<CTOneBase, R>::value)
      return _left();

    else
      return _left() * _right();
  }

  template <CTTag dtag>
  auto D() const
  {
    return _left.template D<dtag>() * _right + _right.template D<dtag>() * _left;
  }

  using typename CTBinary<L, R>::O;
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
  auto operator()() const
  {
    if constexpr (std::is_base_of<CTOneBase, R>::value)
      return _left();

    if constexpr (std::is_base_of<CTNullBase, L>::value && !std::is_base_of<CTNullBase, R>::value)
      return O(0);

    return _left() / _right();
  }

  template <CTTag dtag>
  auto D() const
  {
    return _left.template D<dtag>() / _right -
           _left * _right.template D<dtag>() / (_right * _right);
  }

  using typename CTBinary<L, R>::O;
  using CTBinary<L, R>::_left;
  using CTBinary<L, R>::_right;
};

/**
 * Power operator where both base and exponent can be arbitrary operators.
 */
template <typename L, typename R>
class CTPow : public CTBinary<L, R>
{
public:
  CTPow(L left, R right) : CTBinary<L, R>(left, right) {}
  auto operator()() const
  {
    if constexpr (std::is_base_of<CTNullBase, L>::value)
      return O(0);

    if constexpr (std::is_base_of<CTOneBase, L>::value || std::is_base_of<CTNullBase, R>::value)
      return O(1);

    if constexpr (std::is_base_of<CTOneBase, R>::value)
      return _left();

    return std::pow(_left(), _right());
  }

  template <CTTag dtag>
  auto D() const
  {
    return pow(_left, _right) * _right.template D<dtag>() * log(_left) +
           _right * _left.template D<dtag>() / _left;
  }

  using typename CTBinary<L, R>::O;
  using CTBinary<L, R>::_left;
  using CTBinary<L, R>::_right;
};

/**
 * Integer exponent power operator.
 */
template <typename B, int E>
class CTIPow : public CTUnary<B>
{
public:
  CTIPow(B base) : CTUnary<B>(base) {}
  auto operator()() const
  {
    if constexpr (std::is_base_of<CTNullBase, B>::value)
      return O(0);

    else if constexpr (std::is_base_of<CTOneBase, B>::value || E == 0)
      return O(1);

    else if constexpr (E == 1)
      return _arg();

    else if constexpr (E < 0)
      return 1.0 / libMesh::Utility::pow<-E>(_arg());

    else
      return libMesh::Utility::pow<E>(_arg());
  }

  template <CTTag dtag>
  auto D() const
  {
    if constexpr (E == 1)
      return _arg.template D<dtag>();

    else if constexpr (E == 0)
      return CTNull<O>();

    else
      return pow<E - 1>(_arg) * E * _arg.template D<dtag>();
  }

  using typename CTUnary<B>::O;
  using CTUnary<B>::_arg;
};

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
    if constexpr (std::is_base_of<CTBase, L>::value && std::is_base_of<CTBase, R>::value)          \
      return OP(left, right);                                                                      \
    else if constexpr (std::is_base_of<CTBase, L>::value)                                          \
      return OP(left, CTRef<-1, R>(right));                                                        \
    else if constexpr (std::is_base_of<CTBase, R>::value)                                          \
      return OP(CTRef<-1, L>(left), right);                                                        \
  }                                                                                                \
  template <typename L, typename R, class = std::enable_if_t<!std::is_base_of<CTBase, L>::value>>  \
  auto operator op(const L && left, const R & right)                                               \
  {                                                                                                \
    return OP(CTValue<L>(left), right);                                                            \
  }                                                                                                \
  template <typename L, typename R, class = std::enable_if_t<!std::is_base_of<CTBase, R>::value>>  \
  auto operator op(const L & left, const R && right)                                               \
  {                                                                                                \
    return OP(left, CTValue<R>(right));                                                            \
  }

CT_OPERATOR_BINARY(+, CTAdd)
CT_OPERATOR_BINARY(-, CTSub)
CT_OPERATOR_BINARY(*, CTMul)
CT_OPERATOR_BINARY(/, CTDiv)

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
    using typename CTUnary<T>::O;                                                                  \
    using CTUnary<T>::_arg;                                                                        \
  };                                                                                               \
  template <typename T>                                                                            \
  auto name(const T & v)                                                                           \
  {                                                                                                \
    return CTF##name(v);                                                                           \
  }

CT_SIMPLE_UNARY_FUNCTION(exp, exp(_arg) * _arg.template D<dtag>())
CT_SIMPLE_UNARY_FUNCTION(log, _arg.template D<dtag>() / _arg)
CT_SIMPLE_UNARY_FUNCTION(sin, cos(_arg) * _arg.template D<dtag>())
CT_SIMPLE_UNARY_FUNCTION(cos, -1.0 * sin(_arg) * _arg.template D<dtag>())
CT_SIMPLE_UNARY_FUNCTION(tan, (pow<2>(tan(_arg)) + 1.0) * _arg.template D<dtag>())
CT_SIMPLE_UNARY_FUNCTION(sqrt, 1.0 / (2.0 * sqrt(_arg)) * _arg.template D<dtag>())

/**
 * pow(base, exponent) function overload.
 */
template <typename B, typename E>
auto
pow(const B & base, const E & exp)
{
  return CTPow(base, exp);
}

/**
 * pow<exponent>(base) template for integer powers.
 */
template <int E, typename B>
auto
pow(const B & base)
{
  return CTIPow<B, E>(base);
}

/**
 * Helper function to build a tagged reference to a variable
 */
template <CTTag tag, typename T>
auto
makeRef(const T & ref)
{
  return CTRef<tag, T>(ref);
}

/**
 * Helper function to build a tagged reference to a vector/array entry
 */
template <CTTag tag, typename T, typename I>
auto
makeRef(const T & ref, const I & idx)
{
  return CTArrayRef<tag, T, I>(ref, idx);
}

} // namespace CompileTimeDerivatives
