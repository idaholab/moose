//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LayeredAverageBase.h"

/**
 * Base class for computing layered Volume averages
 */
template <typename BaseType>
class LayeredVolumeAverageBase : public LayeredAverageBase<BaseType>
{
public:
  static InputParameters validParams();
  LayeredVolumeAverageBase(const InputParameters & parameters);

protected:
  virtual Real volume() const override { return this->_current_elem_volume; };
};

template <typename BaseType>
InputParameters
LayeredVolumeAverageBase<BaseType>::validParams()
{
  return LayeredAverageBase<BaseType>::validParams();
}

template <typename BaseType>
LayeredVolumeAverageBase<BaseType>::LayeredVolumeAverageBase(const InputParameters & parameters)
  : LayeredAverageBase<BaseType>(parameters)
{
}
