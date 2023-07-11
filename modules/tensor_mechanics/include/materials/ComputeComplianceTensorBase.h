//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "RankFourTensor.h"

template <bool is_ad>
class ComputeComplianceTensorBaseTempl : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ComputeComplianceTensorBaseTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void computeQpElasticityTensor() = 0;

  /// Base name of the material system
  const std::string _base_name;

  std::string _elasticity_tensor_name;
  std::string _compliance_tensor_name;

  GenericMaterialProperty<RankFourTensor, is_ad> & _compliance_tensor;
  GenericMaterialProperty<RankFourTensor, is_ad> & _elasticity_tensor;
};

typedef ComputeComplianceTensorBaseTempl<false> ComputeComplianceTensorBase;
typedef ComputeComplianceTensorBaseTempl<true>  ADComputeComplianceTensorBase;
