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
 * VolumeDeformGradCorrectedStress transforms the Cauchy stress calculated in the previous
 *configuration to its configuration
 **/
class VolumeDeformGradCorrectedStress : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  VolumeDeformGradCorrectedStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
  virtual void computeQpStress();

  const MaterialProperty<RankTwoTensor> & _pre_stress;
  const MaterialProperty<RankTwoTensor> & _deformation_gradient;
  const MaterialProperty<RankFourTensor> * const _pre_Jacobian_mult;

  /// The stress tensor transformed to the current configuration
  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<RankFourTensor> * const _Jacobian_mult;
};
