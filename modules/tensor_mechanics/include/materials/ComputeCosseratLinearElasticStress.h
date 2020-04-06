//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeCosseratStressBase.h"

/**
 * ComputeCosseratLinearElasticStress computes the Cosserat stress
 * and couple-stress following linear elasticity theory
 * It also sets the d(stress)/d(strain) and d(couple_stress)/d(curvature)
 * tensors appropriately
 */
class ComputeCosseratLinearElasticStress : public ComputeCosseratStressBase
{
public:
  static InputParameters validParams();

  ComputeCosseratLinearElasticStress(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual void computeQpStress() override;

  /// Name of the elasticity tensor material property
  const std::string _elasticity_tensor_name;
  /// Elasticity tensor material property
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
};
