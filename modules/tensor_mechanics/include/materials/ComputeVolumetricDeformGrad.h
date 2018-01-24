/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEVOLUMETRICDEFORMGRAD_H
#define COMPUTEVOLUMETRICDEFORMGRAD_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"
#include "RankTwoTensor.h"

/**
 * ComputeVolumetricDeformGrad is the class to compute volumetric deformation gradient
 * Modification based on pre-multiplication to a deformation gradient
 * Can be used to form a chain of volumetric corections on deformation
 */
class ComputeVolumetricDeformGrad : public DerivativeMaterialInterface<Material>
{
public:
  ComputeVolumetricDeformGrad(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
  virtual void createVolumetricDeformGrad();

  const MaterialProperty<RankTwoTensor> & _pre_deform_grad;
  MaterialProperty<RankTwoTensor> & _volumetric_deform_grad;
  MaterialProperty<RankTwoTensor> & _post_deform_grad;
};

#endif // COMPUTEVOLUMETRICDEFORMGARD_H
