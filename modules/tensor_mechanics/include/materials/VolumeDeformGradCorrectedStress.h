//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VOLUMEDEFORMGRADCORRECTEDSTRESS_H
#define VOLUMEDEFORMGRADCORRECTEDSTRESS_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "RotationTensor.h"
#include "DerivativeMaterialInterface.h"

class VolumeDeformGradCorrectedStress;

template <>
InputParameters validParams<VolumeDeformGradCorrectedStress>();

/**
 * VolumeDeformGradCorrectedStress transforms the Cauchy stress calculated in the previous
 *configuration to its configuration
 **/
class VolumeDeformGradCorrectedStress : public DerivativeMaterialInterface<Material>
{
public:
  VolumeDeformGradCorrectedStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
  virtual void computeQpStress();

  const MaterialProperty<RankTwoTensor> & _pre_stress;
  const MaterialProperty<RankTwoTensor> & _deformation_gradient;
  const MaterialProperty<RankFourTensor> * _pre_Jacobian_mult;

  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<RankFourTensor> * _Jacobian_mult;
};

#endif
