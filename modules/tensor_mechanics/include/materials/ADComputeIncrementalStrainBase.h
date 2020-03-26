//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeStrainBase.h"

/**
 * ADComputeIncrementalStrainBase is the base class for strain tensors using incremental
 * formulations
 */
class ADComputeIncrementalStrainBase : public ADComputeStrainBase
{
public:
  static InputParameters validParams();

  ADComputeIncrementalStrainBase(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void initQpStatefulProperties() override;

  void subtractEigenstrainIncrementFromStrain(ADRankTwoTensor & strain);

  std::vector<const VariableGradient *> _grad_disp_old;

  ADMaterialProperty<RankTwoTensor> & _strain_rate;
  ADMaterialProperty<RankTwoTensor> & _strain_increment;
  ADMaterialProperty<RankTwoTensor> & _rotation_increment;

  const MaterialProperty<RankTwoTensor> & _mechanical_strain_old;
  const MaterialProperty<RankTwoTensor> & _total_strain_old;

  std::vector<const MaterialProperty<RankTwoTensor> *> _eigenstrains_old;
};
