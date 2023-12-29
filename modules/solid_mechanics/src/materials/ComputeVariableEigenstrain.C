//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeVariableEigenstrain.h"

registerMooseObject("TensorMechanicsApp", ComputeVariableEigenstrain);

InputParameters
ComputeVariableEigenstrain::validParams()
{
  InputParameters params = ComputeEigenstrain::validParams();
  params.addClassDescription("Computes an Eigenstrain and its derivatives that is a function of "
                             "multiple variables, where the prefactor is defined in a derivative "
                             "material");
  params.addRequiredCoupledVar("args", "variable dependencies for the prefactor");
  return params;
}

ComputeVariableEigenstrain::ComputeVariableEigenstrain(const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeEigenstrain>(parameters),
    _num_args(coupledComponents("args")),
    _dprefactor(_num_args),
    _d2prefactor(_num_args),
    _delastic_strain(_num_args),
    _d2elastic_strain(_num_args)
{
  // fetch prerequisite derivatives and build elastic_strain derivatives and cross-derivatives
  for (unsigned int i = 0; i < _num_args; ++i)
  {
    const VariableName & iname = coupledName("args", i);
    _dprefactor[i] = &getMaterialPropertyDerivative<Real>("prefactor", iname);
    _delastic_strain[i] =
        &declarePropertyDerivative<RankTwoTensor>(_base_name + "elastic_strain", iname);

    _d2prefactor[i].resize(_num_args);
    _d2elastic_strain[i].resize(_num_args);

    for (unsigned int j = i; j < _num_args; ++j)
    {
      const VariableName & jname = coupledName("args", j);
      _d2prefactor[i][j] = &getMaterialPropertyDerivative<Real>("prefactor", iname, jname);
      _d2elastic_strain[i][j] =
          &declarePropertyDerivative<RankTwoTensor>(_base_name + "elastic_strain", iname, jname);
    }
  }
}

void
ComputeVariableEigenstrain::computeQpEigenstrain()
{
  ComputeEigenstrain::computeQpEigenstrain();

  // Define derivatives of the elastic strain
  for (unsigned int i = 0; i < _num_args; ++i)
  {
    (*_delastic_strain[i])[_qp] = -_eigen_base_tensor * (*_dprefactor[i])[_qp];
    for (unsigned int j = i; j < _num_args; ++j)
      (*_d2elastic_strain[i][j])[_qp] = -_eigen_base_tensor * (*_d2prefactor[i][j])[_qp];
  }
}
