//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodalPatchRecoveryBase.h"

/**
 * Prepare patches for use in nodal patch recovery based on a material property.
 * TODO: Right now this class expects a scalar-valued, i.e. `Real`, material property. If you want
 * to use this userobject to construct patches for a component of a tensor-valued material property,
 * use the `RankTwoCartesianComponent` to first extract the desired component from the rank-2
 * tensor. We need to figure out how to use IndexableProperty in this UO.
 */
class NodalPatchRecoveryMaterialProperty : public NodalPatchRecoveryBase
{
public:
  static InputParameters validParams();

  NodalPatchRecoveryMaterialProperty(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const MaterialProperty<Real> & _prop;
};
