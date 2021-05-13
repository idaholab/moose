//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACBarrierFunction.h"
#include "libmesh/utility.h"

registerMooseObject("PhaseFieldApp", ACBarrierFunction);

InputParameters
ACBarrierFunction::validParams()
{
  InputParameters params = ACGrGrBase::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "gamma", "The interface profile coefficient to use with the kernel");
  params.addClassDescription("Allen-Cahn kernel used when 'mu' is a function of variables");
  return params;
}

ACBarrierFunction::ACBarrierFunction(const InputParameters & parameters)
  : ACGrGrBase(parameters),
    _n_eta(_vals.size()),
    _uname(getParam<NonlinearVariableName>("variable")),
    _gamma_name(getParam<MaterialPropertyName>("gamma")),
    _gamma(getMaterialPropertyByName<Real>(_gamma_name)),
    _dmudvar(getMaterialPropertyDerivative<Real>("mu", _uname)),
    _d2mudvar2(getMaterialPropertyDerivative<Real>("mu", _uname, _uname)),
    _vname(getParam<std::vector<VariableName>>("v")),
    _d2mudvardeta(_n_eta),
    _vmap(getParameterJvarMap("v"))
{
  for (unsigned int i = 0; i < _n_eta; ++i)
    _d2mudvardeta[i] = &getMaterialPropertyDerivative<Real>("mu", _uname, _vname[i]);
}

Real
ACBarrierFunction::computeDFDOP(PFFunctionType type)
{
  switch (type)
  {
    case Residual:
      return _dmudvar[_qp] * calculateF0();

    case Jacobian:
    {
      Real df0dvar = 0.0;
      for (unsigned int i = 0; i < _n_eta; ++i)
        df0dvar += (*_vals[i])[_qp] * (*_vals[i])[_qp];

      df0dvar *= 2.0 * _gamma[_qp] * _u[_qp];
      df0dvar += _u[_qp] * _u[_qp] * _u[_qp] - _u[_qp];

      return (_d2mudvar2[_qp] * calculateF0() + _dmudvar[_qp] * df0dvar) * _phi[_j][_qp];
    }

    default:
      mooseError("Invalid type passed in");
  }
}

Real
ACBarrierFunction::computeQpOffDiagJacobian(unsigned int jvar)
{
  const unsigned int j = mapJvarToCvar(jvar);
  Real sum_etai2 = 0.0;
  Real df0deta_base = 0.0;
  Real df0deta = 0.0;

  for (unsigned int i = 0; i < _n_eta; ++i)
    if (i != j)
      sum_etai2 += (*_vals[i])[_qp] * (*_vals[i])[_qp];

  auto etavar = mapJvarToCvar(jvar, _vmap);
  if (etavar >= 0)
  {
    df0deta_base = (*_vals[etavar])[_qp] * (*_vals[etavar])[_qp] - 1.0 +
                   2.0 * _gamma[_qp] * (_u[_qp] * _u[_qp] + sum_etai2);
    df0deta = (*_vals[etavar])[_qp] * df0deta_base;

    return ((*_d2mudvardeta[etavar])[_qp] * calculateF0() + _dmudvar[_qp] * df0deta) *
           _phi[_j][_qp] * _test[_i][_qp] * _L[_qp];
  }
  return 0.0;
}

Real
ACBarrierFunction::calculateF0()
{
  Real var_phase = 0.25 * _u[_qp] * _u[_qp] * _u[_qp] * _u[_qp] - 0.5 * _u[_qp] * _u[_qp];
  Real eta_phase = 0.0;
  Real eta_interface = 0.0;
  Real sum_eta_j2;

  for (unsigned int i = 0; i < _n_eta; ++i)
  {
    sum_eta_j2 = 0.0;
    for (unsigned int j = i + 1; j < _n_eta; ++j)
      sum_eta_j2 += (*_vals[j])[_qp] * (*_vals[j])[_qp];
    // eta_phase += 0.25 * (*_vals[i])[_qp] * (*_vals[i])[_qp] * (*_vals[i])[_qp] * (*_vals[i])[_qp]
    // -
    //             0.5 * (*_vals[i])[_qp] * (*_vals[i])[_qp];
    eta_phase += 0.25 * Utility::pow<4>((*_vals[i])[_qp]) - 0.5 * Utility::pow<2>((*_vals[i])[_qp]);
    eta_interface +=
        (_u[_qp] * _u[_qp] + sum_eta_j2) * (*_vals[i])[_qp] * (*_vals[i])[_qp] * _gamma[_qp];
  }
  return 0.25 + var_phase + eta_phase + eta_interface;
}
