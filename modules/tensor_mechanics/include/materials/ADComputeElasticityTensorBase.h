//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADCOMPUTEELASTICITYTENSORBASE_H
#define ADCOMPUTEELASTICITYTENSORBASE_H

#include "ADMaterial.h"
#include "RankFourTensor.h"
#include "GuaranteeProvider.h"
#include "DerivativeMaterialPropertyNameInterface.h"

#define usingComputeElasticityTensorBaseMembers                                                    \
  usingMaterialMembers;                                                                            \
  using ADComputeElasticityTensorBase<compute_stage>::_elasticity_tensor_name;                     \
  using ADComputeElasticityTensorBase<compute_stage>::_elasticity_tensor;                          \
  using ADComputeElasticityTensorBase<compute_stage>::issueGuarantee;

template <ComputeStage>
class ADComputeElasticityTensorBase;
template <typename>
class RankFourTensorTempl;
typedef RankFourTensorTempl<Real> RankFourTensor;

declareADValidParams(ADComputeElasticityTensorBase);

/**
 * ADComputeElasticityTensorBase is a the base class for computing elasticity tensors
 */
template <ComputeStage compute_stage>
class ADComputeElasticityTensorBase : public ADMaterial<compute_stage>,
                                      public DerivativeMaterialPropertyNameInterface,
                                      public GuaranteeProvider
{
public:
  ADComputeElasticityTensorBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void computeQpElasticityTensor() = 0;

  std::string _base_name;
  std::string _elasticity_tensor_name;

  ADMaterialProperty(RankFourTensor) & _elasticity_tensor;

  /// prefactor function to multiply the elasticity tensor with
  Function * const _prefactor_function;

  usingMaterialMembers;
};

#endif // ADCOMPUTEELASTICITYTENSORBASE_H
