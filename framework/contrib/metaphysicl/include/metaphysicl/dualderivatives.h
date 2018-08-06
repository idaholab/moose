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


#ifndef METAPHYSICL_DUALDERIVATIVES_H
#define METAPHYSICL_DUALDERIVATIVES_H

#include "metaphysicl/raw_type.h"

namespace MetaPhysicL {

template <typename T>
struct DerivativeType
{
  typedef typename ValueType<typename T::derivatives_type>::type type;
};


template <typename T>
struct DerivativesType
{
  typedef typename T::derivatives_type type;
};


template <typename T>
struct DivergenceType
{
  // The "divergence" of a scalar is a plain derivative
  typedef typename DerivativeType<T>::type type;
};


template <typename T>
inline
typename DerivativeType<T>::type
derivative(const T& a, unsigned int derivativeindex)
{
  return a.derivatives()[derivativeindex];
}

template <typename T>
inline
typename DerivativesType<T>::type
derivatives(const T& a)
{
  return a.derivatives();
}


template <typename T, unsigned int derivativeindex>
struct DerivativeOf {
  static
  typename DerivativeType<T>::type
  derivative(const T& a)
  {
    return a.derivatives().template get<derivativeindex>();
  }
};

} // namespace MetaPhysicL

#endif // METAPHYSICL_DUALDERIVATIVES_H
