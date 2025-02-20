//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LayeredIntegralBase.h"

/**
 * Base class for computing layered Volume Integrals
 */
template <typename BaseType>
class LayeredVolumeIntegralBase : public LayeredIntegralBase<BaseType>
{
public:
  static InputParameters validParams();
  LayeredVolumeIntegralBase(const InputParameters & parameters);

  virtual bool hasBlocks(SubdomainID sub) const override;
};

template <typename BaseType>
InputParameters
LayeredVolumeIntegralBase<BaseType>::validParams()
{
  return LayeredIntegralBase<BaseType>::validParams();
}

template <typename BaseType>
LayeredVolumeIntegralBase<BaseType>::LayeredVolumeIntegralBase(const InputParameters & parameters)
  : LayeredIntegralBase<BaseType>(parameters)
{
}

template <typename UserObjectType>
bool
LayeredVolumeIntegralBase<UserObjectType>::hasBlocks(const SubdomainID sub) const
{
  return UserObjectType::hasBlocks(sub);
}
