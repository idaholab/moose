//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEELASTICITYTENSORBASE_H
#define COMPUTEELASTICITYTENSORBASE_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "RankFourTensor.h"
#include "GuaranteeProvider.h"

class ComputeElasticityTensorBase;

template <>
InputParameters validParams<ComputeElasticityTensorBase>();

/**
 * ComputeElasticityTensorBase the base class for computing elasticity tensors
 */
class ComputeElasticityTensorBase : public DerivativeMaterialInterface<Material>,
                                    public GuaranteeProvider
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
