//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeRotatedElasticityTensorBase.h"

/**
 * ComputeElasticityTensor defines an elasticity tensor material object as a function of
 * concentration field.
 */
class ComputeConcentrationDependentElasticityTensor : public ComputeRotatedElasticityTensorBase
{
public:
  static InputParameters validParams();

  ComputeConcentrationDependentElasticityTensor(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor();

  /// Elasticity tensor for phase with zero concentration.
  RankFourTensor _Cijkl0;
  /// Elasticity tensor for phase with concentration 1.
  RankFourTensor _Cijkl1;
  /// Concentration variable.
  const VariableValue & _c;
  VariableName _c_name;

  /// Derivative of elasticity tensor with respect to concentration.
  MaterialProperty<RankFourTensor> * _delasticity_tensor_dc;
};
