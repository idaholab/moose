/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEELASTICITYTENSORBASE_H
#define COMPUTEELASTICITYTENSORBASE_H

#include "Material.h"
#include "RankFourTensor.h"

/**
 * ComputeElasticityTensorBase the base class for computing elasticity tensors
 */
class ComputeElasticityTensorBase : public DerivativeMaterialInterface<Material>
{
public:
  ComputeElasticityTensorBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void computeQpElasticityTensor() = 0;

  std::string _base_name;
  std::string _elasticity_tensor_name;

  MaterialProperty<RankFourTensor> & _elasticity_tensor;

  /// prefactor function to multiply the elasticity tensor with
  Function * const _prefactor_function;
};

#endif // COMPUTEELASTICITYTENSORBASE_H
