//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "RotationTensor.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeDeformGradBasedStress computes stress based on lagrangian strain definition
 */
class ComputeDeformGradBasedStress : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ComputeDeformGradBasedStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
  virtual void computeQpStress();

  const MaterialProperty<RankTwoTensor> & _deformation_gradient;
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

  /// The stress tensor to be calculated
  MaterialProperty<RankTwoTensor> & _stress;

  MaterialProperty<RankFourTensor> & _Jacobian_mult;
};
