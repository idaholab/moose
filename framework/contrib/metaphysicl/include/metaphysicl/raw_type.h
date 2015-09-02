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
// $Id: core.h 37197 2013-02-21 05:49:09Z roystgnr $
//
//--------------------------------------------------------------------------

#ifndef METAPHYSICL_RAW_TYPE_H
#define METAPHYSICL_RAW_TYPE_H

#include <limits>


namespace MetaPhysicL {

// ValueType strips the dimensionality off of 
// vector types.  Differentiable types remain differentiable.
template <typename T>
struct ValueType
{
  typedef T type;
};

template <typename T>
struct ValueType<const T>
{
  typedef const typename ValueType<T>::type type;
};

// RawType strips the derivatives, shadow magic, etc. off of 
// types.  Vector types remain vector types.
template <typename T>
struct RawType
{
  typedef T value_type;

  static value_type value(const T& a) { return a; }
};

template <typename T>
struct RawType<const T>
{
  typedef const typename RawType<T>::value_type value_type;

  static value_type value(const T& a) { return RawType<T>::value(a); }
};


// Make the user syntax slightly nicer
template <typename T>
inline
typename RawType<T>::value_type
raw_value(const T& a) { return RawType<T>::value(a); }


template <typename NewType, typename OldType>
class raw_numeric_limits
{
public:
  static const bool is_specialized = true;
  static NewType min() throw() { return NewType(std::numeric_limits<OldType>::min()); }
  static NewType max() throw() { return NewType(std::numeric_limits<OldType>::max()); }
  static const int  digits = std::numeric_limits<OldType>::digits;
  static const int  digits10 = std::numeric_limits<OldType>::digits10;
  static const bool is_signed = std::numeric_limits<OldType>::is_signed;
  static const bool is_integer = std::numeric_limits<OldType>::is_integer;
  static const bool is_exact = std::numeric_limits<OldType>::is_exact;
  static const int radix = std::numeric_limits<OldType>::radix;
  static NewType epsilon() throw() {return NewType(std::numeric_limits<OldType>::epsilon()); }
  static NewType round_error() throw() {return NewType(std::numeric_limits<OldType>::round_error()); }

  static const int  min_exponent = std::numeric_limits<OldType>::min_exponent;
  static const int  min_exponent10 = std::numeric_limits<OldType>::min_exponent10;
  static const int  max_exponent = std::numeric_limits<OldType>::max_exponent;
  static const int  max_exponent10 = std::numeric_limits<OldType>::max_exponent10;

  static const bool has_infinity = std::numeric_limits<OldType>::has_infinity;
  static const bool has_quiet_NaN = std::numeric_limits<OldType>::has_quiet_NaN;
  static const bool has_signaling_NaN = std::numeric_limits<OldType>::has_signaling_NaN;
  static const std::float_denorm_style has_denorm = std::numeric_limits<OldType>::has_denorm;
  static const bool has_denorm_loss = std::numeric_limits<OldType>::has_denorm_loss;
  static NewType infinity() throw() {return NewType(std::numeric_limits<OldType>::infinity()); }
  static NewType quiet_NaN() throw() {return NewType(std::numeric_limits<OldType>::quiet_NaN()); }
  static NewType signaling_NaN() throw() {return NewType(std::numeric_limits<OldType>::signaling_NaN()); }
  static NewType denorm_min() throw() {return NewType(std::numeric_limits<OldType>::denorm_min()); }

  static const bool is_iec559 = std::numeric_limits<OldType>::is_iec559;
  static const bool is_bounded = std::numeric_limits<OldType>::is_bounded;
  static const bool is_modulo = std::numeric_limits<OldType>::is_modulo;

  static const bool traps = std::numeric_limits<OldType>::traps;
  static const bool tinyness_before = std::numeric_limits<OldType>::tinyness_before;
  static const std::float_round_style round_style = std::numeric_limits<OldType>::round_style;
};


} // namespace MetaPhysicL

#endif // METAPHYSICL_RAW_TYPE_H
