//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrainBase.h"
#include "RankFourTensor.h"

class ComputeEigenstrainFromInitialStress;

template <>
InputParameters validParams<ComputeEigenstrainFromInitialStress>();

/**
 * ComputeEigenstrain computes an Eigenstrain that results from an initial stress
 * The initial stress is defined in terms of Functions, which may be
 * multiplied by optional AuxVariables
 */
class ComputeEigenstrainFromInitialStress : public ComputeEigenstrainBase
{
public:
  ComputeEigenstrainFromInitialStress(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;

  /// base_name for elasticity tensor to use to convert stress to strain
  const std::string _base_name;

  /// elasticity tensor used to convert stress to strain
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

  ///Stores the total eigenstrain in the previous step
  const MaterialProperty<RankTwoTensor> & _eigenstrain_old;

  /// Whether the user has supplied AuxVariables representing the initial stress
  const bool _ini_aux_provided;

  /// initial stress components
  std::vector<Function *> _initial_stress_fcn;

  /// AuxVariables defining the initial stress
  std::vector<const VariableValue *> _ini_aux;
};

