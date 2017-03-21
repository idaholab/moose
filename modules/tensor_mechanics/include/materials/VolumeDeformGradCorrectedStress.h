/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef VOLUMEDEFORMGRADCORRECTEDSTRESS_H
#define VOLUMEDEFORMGRADCORRECTEDSTRESS_H

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
