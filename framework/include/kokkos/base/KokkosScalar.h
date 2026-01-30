//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosReferenceWrapper.h"

namespace Moose::Kokkos
{

/**
 * The Kokkos wrapper class that can hold the reference of an arithmetic scalar variable
 */
template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
class Scalar : public ReferenceWrapper<T>
{
public:
  /**
   * Constructor
   * @param value The writeable reference of the arithmetic scalar variable to store
   */
  Scalar(T & value) : ReferenceWrapper<T>(value) {}

  /**
   * Assign a scalar value to the underlying host reference
   * @param value The scalar value to be assigned
   */
  auto & operator=(T value)
  {
    this->_reference = value;

    return *this;
  }

  // TODO: add support for arithmetic operators
};

template <typename T>
struct ArrayDeepCopy<Scalar<T>>
{
  static constexpr bool value = true;
};

// Mimic MOOSE convention
using PostprocessorValue = Scalar<const PostprocessorValue>;

} // namespace Moose::Kokkos
