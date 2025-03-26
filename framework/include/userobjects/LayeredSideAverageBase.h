//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LayeredAverageBase.h"

/**
 * Base class for computing layered side averages
 */
template <typename BaseType>
class LayeredSideAverageBase : public LayeredAverageBase<BaseType>
{
public:
  static InputParameters validParams();
  LayeredSideAverageBase(const InputParameters & parameters);

protected:
  virtual Real volume() const override { return this->_current_side_volume; };
};

template <typename BaseType>
InputParameters
LayeredSideAverageBase<BaseType>::validParams()
{
  return LayeredAverageBase<BaseType>::validParams();
}

template <typename BaseType>
LayeredSideAverageBase<BaseType>::LayeredSideAverageBase(const InputParameters & parameters)
  : LayeredAverageBase<BaseType>(parameters)
{
}
