//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUReferenceWrapper.h"

// TODO: add support for arithmetic operators

namespace Moose
{
namespace Kokkos
{

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
class Scalar : public ReferenceWrapper<T>
{
public:
  Scalar(T & value) : ReferenceWrapper<T>(value) {}

  auto & operator=(T value)
  {
    this->_reference = value;

    return *this;
  }
};

template <typename T>
class Scalar<const T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
  : public ReferenceWrapper<const T>
{
public:
  Scalar(const T & value) : ReferenceWrapper<const T>(value) {}
};

using PostprocessorValue = Scalar<const PostprocessorValue>;

} // namespace Kokkos
} // namespace Moose
