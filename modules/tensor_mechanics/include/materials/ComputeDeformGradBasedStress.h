/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEDEFORMGRADBASEDSTRESS_H
#define COMPUTEDEFORMGRADBASEDSTRESS_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "ElasticityTensorR4.h"
#include "RotationTensor.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeDeformGradBasedStress computes stress based on lagrangian strain definition
 **/
class ComputeDeformGradBasedStress : public DerivativeMaterialInterface<Material>
{
public:
  ComputeDeformGradBasedStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
  virtual void computeQpStress();

  const MaterialProperty<RankTwoTensor> & _deformation_gradient;
  const MaterialProperty<ElasticityTensorR4> & _elasticity_tensor;

  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<ElasticityTensorR4> & _Jacobian_mult;
};

#endif
