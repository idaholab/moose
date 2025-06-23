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

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
class GPUScalar : public GPUReferenceWrapper<T>
{
public:
  GPUScalar(T & value) : GPUReferenceWrapper<T>(value) {}

  auto & operator=(T value)
  {
    this->_reference = value;

    return *this;
  }
};

template <typename T>
class GPUScalar<const T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
  : public GPUReferenceWrapper<const T>
{
public:
  GPUScalar(const T & value) : GPUReferenceWrapper<const T>(value) {}
};

using GPUPostprocessorValue = GPUScalar<const PostprocessorValue>;
