/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeVariableEigenstrain.h"

template <>
InputParameters
validParams<ComputeVariableEigenstrain>()
{
  InputParameters params = validParams<ComputeEigenstrain>();
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
    const VariableName & iname = getVar("args", i)->name();
    _dprefactor[i] = &getMaterialPropertyDerivative<Real>("prefactor", iname);
    _delastic_strain[i] =
        &declarePropertyDerivative<RankTwoTensor>(_base_name + "elastic_strain", iname);

    _d2prefactor[i].resize(_num_args);
    _d2elastic_strain[i].resize(_num_args);

    for (unsigned int j = i; j < _num_args; ++j)
    {
      const VariableName & jname = getVar("args", j)->name();
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
