//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterialPropertyValueDecl.h"
#include "KokkosMaterialPropertyDecl.h"
#include "KokkosDatum.h"

namespace Moose::Kokkos
{

template <typename T, unsigned int dimension>
KOKKOS_FUNCTION
MaterialPropertyValueBase<T, dimension>::MaterialPropertyValueBase(
    const MaterialProperty<T, dimension> & property, const Datum & datum, const unsigned int qp)
  : _idx(datum.propertyIdx(property._constant_option, qp)),
    _data(property._default ? nullptr : &property._data[datum.subdomain()]),
    _value(property._value)
{
}

template <typename T>
MaterialPropertyValue<T, 0>::MaterialPropertyValue(const MaterialProperty<T, 0> & property,
                                                   const Datum & datum,
                                                   unsigned int qp)
  : MaterialPropertyValueBase<T, 0>(property, datum, qp)
{
}

template <typename T>
MaterialPropertyValue<T, 1>::MaterialPropertyValue(const MaterialProperty<T, 1> & property,
                                                   const Datum & datum,
                                                   unsigned int qp)
  : MaterialPropertyValueBase<T, 1>(property, datum, qp)
{
}

template <typename T>
MaterialPropertyValue<T, 2>::MaterialPropertyValue(const MaterialProperty<T, 2> & property,
                                                   const Datum & datum,
                                                   unsigned int qp)
  : MaterialPropertyValueBase<T, 2>(property, datum, qp)
{
}

template <typename T>
MaterialPropertyValue<T, 3>::MaterialPropertyValue(const MaterialProperty<T, 3> & property,
                                                   const Datum & datum,
                                                   unsigned int qp)
  : MaterialPropertyValueBase<T, 3>(property, datum, qp)
{
}

template <typename T>
MaterialPropertyValue<T, 4>::MaterialPropertyValue(const MaterialProperty<T, 4> & property,
                                                   const Datum & datum,
                                                   unsigned int qp)
  : MaterialPropertyValueBase<T, 4>(property, datum, qp)
{
}

template <typename T>
KOKKOS_FUNCTION auto &
MaterialPropertyValue<T, 0>::operator=(const T & value)
{
  (*_data)(_idx) = value;

  return *this;
}

template <typename T>
KOKKOS_FUNCTION auto &
MaterialPropertyValue<T, 0>::operator=(const MaterialPropertyValue<T, 0> & value)
{
  (*_data)(_idx) = static_cast<T>(value);

  return *this;
}

} // namespace Moose::Kokkos
