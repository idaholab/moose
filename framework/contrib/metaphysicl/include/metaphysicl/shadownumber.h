//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
// 
// MetaPhysicL - A metaprogramming library for physics calculations
//
// Copyright (C) 2013 The PECOS Development Team
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the Version 2.1 GNU Lesser General
// Public License as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc. 51 Franklin Street, Fifth Floor,
// Boston, MA  02110-1301  USA
//
//-----------------------------------------------------------------------el-
//
// $Id$
//
//--------------------------------------------------------------------------


#ifndef METAPHYSICL_SHADOWNUMBER_H
#define METAPHYSICL_SHADOWNUMBER_H

#include <algorithm>
#include <iostream>
#include <limits>

#include "metaphysicl/compare_types.h"
#include "metaphysicl/raw_type.h"
#include "metaphysicl/testable.h"

namespace MetaPhysicL {

template <typename T, typename S>
class ShadowNumber : public safe_bool<ShadowNumber<T,S> >
{
public:
  typedef T value_type;

  typedef S shadow_type;

  ShadowNumber() {}

  template <typename T2, typename S2>
  ShadowNumber(const T2& v, const S2& s) : _val(v), _shadow(s) {}

  template <typename T2>
  ShadowNumber(const T2& val) : _val(val), _shadow(val) {}

  template <typename T2, typename S2>
  ShadowNumber(ShadowNumber<T2, S2>& other) : _val(other._val), _shadow(other._shadow) {}

  T& value() { return _val; }

  const T& value() const { return _val; }

  S& shadow() { return _shadow; }

  const S& shadow() const { return _shadow; }

  bool boolean_test() const { return _val; }

  ShadowNumber<T,S> operator- () const { return ShadowNumber<T,S> (-_val, -_shadow); }

  ShadowNumber<T,S> operator! () const { return ShadowNumber<T,S> (!_val, !_shadow); }

  template <typename T2, typename S2>
  ShadowNumber<T,S>& operator+= (const ShadowNumber<T2,S2>& a)
    { _val += a.value(); _shadow += a.shadow(); return *this; }

  template <typename T2>
  ShadowNumber<T,S>& operator+= (const T2& a)
    { _val += a; _shadow += a; return *this; }

  template <typename T2, typename S2>
  ShadowNumber<T,S>& operator-= (const ShadowNumber<T2,S2>& a)
    { _val -= a.value(); _shadow -= a.shadow(); return *this; }

  template <typename T2>
  ShadowNumber<T,S>& operator-= (const T2& a)
    { _val -= a; _shadow -= a; return *this; }

  template <typename T2, typename S2>
  ShadowNumber<T,S>& operator*= (const ShadowNumber<T2,S2>& a)
    { _val *= a.value(); _shadow *= a.shadow(); return *this; }

  template <typename T2>
  ShadowNumber<T,S>& operator*= (const T2& a)
    { _val *= a; _shadow *= a; return *this; }

  template <typename T2, typename S2>
  ShadowNumber<T,S>& operator/= (const ShadowNumber<T2,S2>& a)
    { _val /= a.value(); _shadow /= a.shadow(); return *this; }

  template <typename T2>
  ShadowNumber<T,S>& operator/= (const T2& a)
    { _val /= a; _shadow /= a; return *this; }

private:
  T _val;
  S _shadow;
};

//
// Non-member functions
//

#define ShadowNumber_op(opname) \
template <typename T, typename S, typename T2, typename S2> \
inline \
typename CompareTypes<ShadowNumber<T,S>,ShadowNumber<T2,S2> >::supertype \
operator opname (const ShadowNumber<T,S>& a, const ShadowNumber<T2,S2>& b) \
{ \
  typedef typename CompareTypes<ShadowNumber<T,S>,ShadowNumber<T2,S2> >::supertype TS; \
  TS returnval(a); \
  returnval opname##= b; \
  return returnval; \
} \
 \
template <typename T, typename S, typename T2> \
inline \
typename CompareTypes<ShadowNumber<T,S>,T2>::supertype \
operator opname (const ShadowNumber<T,S>& a, const T2& b) \
{ \
  typedef typename CompareTypes<ShadowNumber<T,S>,T2>::supertype TS; \
  TS returnval(a); \
  returnval opname##= b; \
  return returnval; \
 \
} \
template <typename T, typename T2, typename S> \
inline \
typename CompareTypes<ShadowNumber<T2,S>,T>::supertype \
operator opname (const T& a, const ShadowNumber<T2,S>& b) \
{ \
  typedef typename CompareTypes<ShadowNumber<T2,S>,T>::supertype TS; \
  TS returnval(a); \
  returnval opname##= b; \
  return returnval; \
}

ShadowNumber_op(+)
ShadowNumber_op(-)
ShadowNumber_op(*)
ShadowNumber_op(/)

#define ShadowNumber_operator_binary(opname) \
template <typename T, typename S, typename T2, typename S2> \
inline \
ShadowNumber<bool, bool> \
operator opname (const ShadowNumber<T,S>& a, const ShadowNumber<T2,S2>& b) \
{ \
  return ShadowNumber<bool, bool> (a.value() opname b.value(), a.shadow() opname b.shadow()); \
} \
 \
template <typename T, typename S, typename T2> \
inline \
typename boostcopy::enable_if_class< \
  typename CompareTypes<ShadowNumber<T,S>,T2>::supertype, \
  ShadowNumber<bool,bool> \
>::type \
operator opname (const ShadowNumber<T,S>& a, const T2& b) \
{ \
  return ShadowNumber<bool, bool> (a.value() opname b, a.shadow() opname b); \
} \
 \
template <typename T, typename T2, typename S> \
inline \
typename boostcopy::enable_if_class< \
  typename CompareTypes<ShadowNumber<T2,S>,T>::supertype, \
  ShadowNumber<bool,bool> \
>::type \
operator opname (const T& a, const ShadowNumber<T2,S>& b) \
{ \
  return ShadowNumber<bool, bool> (a opname b.value(), a opname b.shadow()); \
}


ShadowNumber_operator_binary(<)
ShadowNumber_operator_binary(<=)
ShadowNumber_operator_binary(>)
ShadowNumber_operator_binary(>=)
ShadowNumber_operator_binary(==)
ShadowNumber_operator_binary(!=)

template <typename T, typename S>
inline
std::ostream&      
operator<< (std::ostream& output, const ShadowNumber<T,S>& a)
{
  return output << '(' << a.value() << ',' << a.shadow() << ')';
}


// ScalarTraits, RawType, CompareTypes specializations

template <typename T, typename S>
struct ScalarTraits<ShadowNumber<T, S> >
{
  static const bool value = ScalarTraits<T>::value;
};

#define ShadowNumber_comparisons(templatename) \
template<typename T, typename S, bool reverseorder> \
struct templatename<ShadowNumber<T,S>, ShadowNumber<T,S>, reverseorder> { \
  typedef ShadowNumber<T, S> supertype; \
}; \
 \
template<typename T, typename S, typename T2, typename S2, bool reverseorder> \
struct templatename<ShadowNumber<T,S>, ShadowNumber<T2,S2>, reverseorder> { \
  typedef ShadowNumber<typename Symmetric##templatename<T, T2, reverseorder>::supertype, \
                       typename Symmetric##templatename<S, S2, reverseorder>::supertype> supertype; \
}; \
 \
template<typename T, typename S, typename T2, bool reverseorder> \
struct templatename<ShadowNumber<T, S>, T2, reverseorder, \
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> { \
  typedef ShadowNumber<typename Symmetric##templatename<T, T2, reverseorder>::supertype, \
                       typename Symmetric##templatename<S, T2, reverseorder>::supertype> supertype; \
}

ShadowNumber_comparisons(CompareTypes);
ShadowNumber_comparisons(PlusType);
ShadowNumber_comparisons(MinusType);
ShadowNumber_comparisons(MultipliesType);
ShadowNumber_comparisons(DividesType);
ShadowNumber_comparisons(AndType);
ShadowNumber_comparisons(OrType);



template <typename T, typename S>
struct RawType<ShadowNumber<T, S> >
{
  typedef typename RawType<T>::value_type value_type;

  static value_type value(const ShadowNumber<T, S>& a) {
    const S max_value = std::max(S(a.value()), a.shadow());
    if (max_value) {
      const S relative_error = (a.value() - a.shadow()) / max_value;
      if (relative_error > 10*std::numeric_limits<T>::epsilon())
        std::cerr << "Shadow relative error = " << relative_error << std::endl;
    }
    return a.value();
  }
};

} // namespace MetaPhysicL


namespace std {

using MetaPhysicL::CompareTypes;
using MetaPhysicL::ShadowNumber;

// Now just combined declaration/definitions

#define ShadowNumber_std_unary(funcname) \
template <typename T, typename S> \
inline \
ShadowNumber<T, S> \
funcname (ShadowNumber<T, S> a) \
{ \
  a.value() = std::funcname(a.value()); \
  a.shadow() = std::funcname(a.shadow()); \
  return a; \
}


#define ShadowNumber_std_binary(funcname) \
template <typename T, typename S, typename T2, typename S2> \
inline \
typename CompareTypes<ShadowNumber<T,S>,ShadowNumber<T2,S2> >::supertype \
funcname (const ShadowNumber<T,S>& a, const ShadowNumber<T2,S2>& b) \
{ \
  typedef typename CompareTypes<ShadowNumber<T,S>,ShadowNumber<T2,S2> >::supertype TS; \
  return TS (std::funcname(a.value(), b.value()), \
             std::funcname(a.shadow(), b.shadow())); \
} \
 \
template <typename T, typename S> \
inline \
ShadowNumber<T,S> \
funcname (const ShadowNumber<T,S>& a, const ShadowNumber<T,S>& b) \
{ \
  return ShadowNumber<T,S> \
    (std::funcname(a.value(), b.value()), \
     std::funcname(a.shadow(), b.shadow())); \
} \
 \
template <typename T, typename S, typename T2> \
inline \
typename CompareTypes<ShadowNumber<T,S>,T2>::supertype \
funcname (const ShadowNumber<T,S>& a, const T2& b) \
{ \
  typedef typename CompareTypes<ShadowNumber<T,S>,T2>::supertype TS; \
  return TS (std::funcname(a.value(), b), \
             std::funcname(a.shadow(), b)); \
} \
 \
template <typename T, typename T2, typename S> \
inline \
typename CompareTypes<ShadowNumber<T2,S>,T>::supertype \
funcname (const T& a, const ShadowNumber<T2,S>& b) \
{ \
  typedef typename CompareTypes<ShadowNumber<T2,S>,T>::supertype TS; \
  return TS (std::funcname(a, b.value()), \
             std::funcname(a, b.shadow())); \
}

ShadowNumber_std_binary(pow)
ShadowNumber_std_unary(exp)
ShadowNumber_std_unary(log)
ShadowNumber_std_unary(log10)
ShadowNumber_std_unary(sin)
ShadowNumber_std_unary(cos)
ShadowNumber_std_unary(tan)
ShadowNumber_std_unary(asin)
ShadowNumber_std_unary(acos)
ShadowNumber_std_unary(atan)
ShadowNumber_std_binary(atan2)
ShadowNumber_std_unary(sinh)
ShadowNumber_std_unary(cosh)
ShadowNumber_std_unary(tanh)
ShadowNumber_std_unary(sqrt)
ShadowNumber_std_unary(abs)
ShadowNumber_std_unary(fabs)
ShadowNumber_std_binary(max)
ShadowNumber_std_binary(min)
ShadowNumber_std_unary(ceil)
ShadowNumber_std_unary(floor)
ShadowNumber_std_binary(fmod)

template <typename T, typename S>
class numeric_limits<ShadowNumber<T, S> > :
  public MetaPhysicL::raw_numeric_limits<ShadowNumber<T, S>, T> {};

} // namespace std



#endif // METAPHYSICL_SHADOWNUMBER_H
