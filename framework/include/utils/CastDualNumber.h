//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 * no-op cast. Useful for templated wrapper types that resolve to the same type.
 */
template <typename T1, typename T2>
inline typename std::enable_if<std::is_same<T1, T2>::value, T1>::type
dual_number_cast(const T2 & v)
{
  return v;
}

/**
 * Casting to the value type of the passed in dual number just removes derivatives.
 * This allows casting ChainedReal to Real and ChainedADReal to ADReal.
 */
template <typename T1, typename T2>
inline typename std::enable_if<std::is_same<typename T2::value_type, T1>::value, T1>::type
dual_number_cast(const T2 & v)
{
  return v.value();
}
