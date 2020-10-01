//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACMultiInterface.h"

// MOOSE includes
#include "MooseVariable.h"
#include "NonlinearSystem.h"

registerMooseObject("PhaseFieldApp", ACMultiInterface);

InputParameters
ACMultiInterface::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Gradient energy Allen-Cahn Kernel with cross terms");
  params.addRequiredCoupledVar("etas", "All eta_i order parameters of the multiphase problem");
  params.addRequiredParam<std::vector<MaterialPropertyName>>("kappa_names",
                                                             "The kappa used with the kernel");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  return params;
}

ACMultiInterface::ACMultiInterface(const InputParameters & parameters)
  : Kernel(parameters),
    _num_etas(coupledComponents("etas")),
    _eta(coupledValues("etas")),
    _grad_eta(coupledGradients("etas")),
    _eta_vars(_fe_problem.getNonlinearSystemBase().nVariables(), -1),
    _kappa_names(getParam<std::vector<MaterialPropertyName>>("kappa_names")),
    _kappa(_num_etas),
    _L(getMaterialProperty<Real>("mob_name"))
{
  if (_num_etas != _kappa_names.size())
    paramError("kappa_names", "Supply the same number of etas and kappa_names.");

  unsigned int nvariables = _fe_problem.getNonlinearSystemBase().nVariables();

  int a = -1;
  for (unsigned int i = 0; i < _num_etas; ++i)
  {
    // populate lookup table form jvar to _eta index
    unsigned int var = coupled("etas", i);
    if (var < nvariables)
      _eta_vars[var] = i;

    // get the index of the variable the kernel is operating on
    if (coupled("etas", i) == _var.number())
      a = i;

    // get gradient prefactors
    _kappa[i] = &getMaterialPropertyByName<Real>(_kappa_names[i]);
  }

  if (a < 0)
    paramError(
        "etas", "Kernel variable must be listed in etas for ACMultiInterface kernel ", name());
  else
    _a = a;
}

Real
ACMultiInterface::computeQpResidual()
{
  const VariableValue & _eta_a = _u;
  const VariableGradient & _grad_eta_a = _grad_u;

  Real sum = 0.0;
  for (unsigned int b = 0; b < _num_etas; ++b)
  {
    // skip the diagonal term (does not contribute)
    if (b == _a)
      continue;

    sum += (*_kappa[b])[_qp] *
           (
               // order 1 terms
               2.0 * _test[_i][_qp] *
                   (_eta_a[_qp] * (*_grad_eta[b])[_qp] - (*_eta[b])[_qp] * _grad_eta_a[_qp]) *
                   (*_grad_eta[b])[_qp]
               // volume terms
               + (-(_eta_a[_qp] * (*_eta[b])[_qp] * _grad_test[_i][_qp] +
                    _test[_i][_qp] * (*_eta[b])[_qp] * _grad_eta_a[_qp] +
                    _test[_i][_qp] * _eta_a[_qp] * (*_grad_eta[b])[_qp]) *
                  (*_grad_eta[b])[_qp]) -
               (-((*_eta[b])[_qp] * (*_eta[b])[_qp] * _grad_test[_i][_qp] +
                  2.0 * _test[_i][_qp] * (*_eta[b])[_qp] * (*_grad_eta[b])[_qp]) *
                _grad_eta_a[_qp]));
  }

  return _L[_qp] * sum;
}

Real
ACMultiInterface::computeQpJacobian()
{
  Real sum = 0.0;
  for (unsigned int b = 0; b < _num_etas; ++b)
  {
    // skip the diagonal term (does not contribute)
    if (b == _a)
      continue;

    sum += (*_kappa[b])[_qp] *
           (2.0 * _test[_i][_qp] *
                ((_phi[_j][_qp] * (*_grad_eta[b])[_qp] - (*_eta[b])[_qp] * _grad_phi[_j][_qp]) *
                 (*_grad_eta[b])[_qp]) +
            (-(_phi[_j][_qp] * (*_eta[b])[_qp] * _grad_test[_i][_qp] +
               _test[_i][_qp] * (*_eta[b])[_qp] * _grad_phi[_j][_qp] +
               _test[_i][_qp] * _phi[_j][_qp] * (*_grad_eta[b])[_qp]) *
             (*_grad_eta[b])[_qp]) -
            (-((*_eta[b])[_qp] * (*_eta[b])[_qp] * _grad_test[_i][_qp] +
               2.0 * _test[_i][_qp] * (*_eta[b])[_qp] * (*_grad_eta[b])[_qp]) *
             _grad_phi[_j][_qp]));
  }

  return _L[_qp] * sum;
}

Real
ACMultiInterface::computeQpOffDiagJacobian(unsigned int jvar)
{
  const VariableValue & _eta_a = _u;
  const VariableGradient & _grad_eta_a = _grad_u;

  const int b = _eta_vars[jvar];
  if (b < 0)
    return 0.0;

  return _L[_qp] * (*_kappa[b])[_qp] *
         (2.0 * _test[_i][_qp] *
              ((_eta_a[_qp] * _grad_phi[_j][_qp] - _phi[_j][_qp] * _grad_eta_a[_qp]) *
                   (*_grad_eta[b])[_qp] +
               (_eta_a[_qp] * (*_grad_eta[b])[_qp] - (*_eta[b])[_qp] * _grad_eta_a[_qp]) *
                   _grad_phi[_j][_qp]) +
          (-(_eta_a[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp] +
             _test[_i][_qp] * _phi[_j][_qp] * _grad_eta_a[_qp] +
             _test[_i][_qp] * _eta_a[_qp] * _grad_phi[_j][_qp]) *
               (*_grad_eta[b])[_qp] -
           (_eta_a[_qp] * (*_eta[b])[_qp] * _grad_test[_i][_qp] +
            _test[_i][_qp] * (*_eta[b])[_qp] * _grad_eta_a[_qp] +
            _test[_i][_qp] * _eta_a[_qp] * (*_grad_eta[b])[_qp]) *
               _grad_phi[_j][_qp]) -
          (-(2.0 * (*_eta[b])[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp] +
             2.0 * _test[_i][_qp] *
                 (_phi[_j][_qp] * (*_grad_eta[b])[_qp] + (*_eta[b])[_qp] * _grad_phi[_j][_qp])) *
           _grad_eta_a[_qp]));
}
