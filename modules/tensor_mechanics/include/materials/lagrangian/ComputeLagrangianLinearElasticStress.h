//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianObjectiveStress.h"

/// Calculate a small strain elastic stress update
///
/// small_stress = C : mechanical_strain
/// with C the elasticity tensor
///
class ComputeLagrangianLinearElasticStress : public ComputeLagrangianObjectiveStress
{
public:
  static InputParameters validParams();
  ComputeLagrangianLinearElasticStress(const InputParameters & parameters);

protected:
  /// Implement the elastic small stress update
  virtual void computeQpSmallStress();

protected:
  /// The elasticity tensor
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
};
