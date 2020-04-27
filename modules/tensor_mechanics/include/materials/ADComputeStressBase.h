//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"
#include "Function.h"
#include "ADRankTwoTensorForward.h"
#include "ADRankFourTensorForward.h"

/**
 * ADComputeStressBase is the base class for stress tensors
 */
class ADComputeStressBase : public ADMaterial
{
public:
  static InputParameters validParams();

  ADComputeStressBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;
  virtual void computeQpStress() = 0;

  /// Base name of the material system
  const std::string _base_name;

  const ADMaterialProperty<RankTwoTensor> & _mechanical_strain;

  /// The stress tensor to be calculated
  ADMaterialProperty<RankTwoTensor> & _stress;
  ADMaterialProperty<RankTwoTensor> & _elastic_strain;

  /// Extra stress tensors
  std::vector<const MaterialProperty<RankTwoTensor> *> _extra_stresses;

  /// initial stress components
  std::vector<const Function *> _initial_stress_fcn;
};
