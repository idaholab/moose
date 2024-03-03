//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrain.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeVariableEigenstrain computes an Eigenstrain that is a function of
 * variables defined by a base tensor and a scalar function defined in a Derivative Material.
 */
class ComputeVariableEigenstrain : public DerivativeMaterialInterface<ComputeEigenstrain>
{
public:
  static InputParameters validParams();

  ComputeVariableEigenstrain(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain();

  /// number of variables the prefactor depends on
  const unsigned int _num_args;

  /// first derivatives of the prefactor w.r.t. to the args
  std::vector<const MaterialProperty<Real> *> _dprefactor;
  /// second derivatives of the prefactor w.r.t. to the args
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2prefactor;

  /// first derivatives of the elastic strain w.r.t. to the args
  std::vector<MaterialProperty<RankTwoTensor> *> _delastic_strain;
  /// second derivatives of the elastic strain w.r.t. to the args
  std::vector<std::vector<MaterialProperty<RankTwoTensor> *>> _d2elastic_strain;
};
