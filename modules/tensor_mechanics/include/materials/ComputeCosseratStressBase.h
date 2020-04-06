//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeStressBase.h"

/**
 * ComputeCosseratStressBase is the base class for stress tensors
 */
class ComputeCosseratStressBase : public ComputeStressBase
{
public:
  static InputParameters validParams();

  ComputeCosseratStressBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpStress() = 0;

  /// The Cosserat curvature strain
  const MaterialProperty<RankTwoTensor> & _curvature;

  /// The Cosserat elastic flexural rigidity tensor
  const MaterialProperty<RankFourTensor> & _elastic_flexural_rigidity_tensor;

  /// the Cosserat couple-stress
  MaterialProperty<RankTwoTensor> & _stress_couple;

  /// derivative of couple-stress w.r.t. curvature
  MaterialProperty<RankFourTensor> & _Jacobian_mult_couple;
};
