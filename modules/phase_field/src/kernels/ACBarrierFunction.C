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
registerMooseObject("PhaseFieldApp", ADACBarrierFunction);

template <bool is_ad>
InputParameters
ACBarrierFunctionTempl<is_ad>::validParams()
{
  InputParameters params = ACBarrierFunctionBase<is_ad>::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "gamma", "The interface profile coefficient to use with the kernel");
  params.addClassDescription("Allen-Cahn kernel used when 'mu' is a function of variables");
  return params;
}

template <bool is_ad>
ACBarrierFunctionTempl<is_ad>::ACBarrierFunctionTempl(const InputParameters & parameters)
  : ACBarrierFunctionBase<is_ad>(parameters),
    _uname(this->template getParam<NonlinearVariableName>("variable")),
    _gamma_name(this->template getParam<MaterialPropertyName>("gamma")),
    _gamma(this->template getGenericMaterialProperty<Real, is_ad>(_gamma_name)),
    _dmudvar(this->template getGenericMaterialProperty<Real, is_ad>(
        this->derivativePropertyNameFirst("mu", _uname)))
{
}

ACBarrierFunction::ACBarrierFunction(const InputParameters & parameters)
  : ACBarrierFunctionTempl<false>(parameters),
    _n_eta(_vals.size()),
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

ADReal
ADACBarrierFunction::computeDFDOP()
{
  ADReal f0 = 0.25 + 0.25 * _u[_qp] * _u[_qp] * _u[_qp] * _u[_qp] - 0.5 * _u[_qp] * _u[_qp];
  ADReal sum_etaj = _u[_qp] * _u[_qp];

  for (unsigned int i = 0; i < _op_num; ++i)
  {
    for (unsigned int j = i + 1; j < _op_num; ++j)
      sum_etaj += (*_vals[j])[_qp] * (*_vals[j])[_qp];

    f0 += 0.25 * Utility::pow<4>((*_vals[i])[_qp]) - 0.5 * Utility::pow<2>((*_vals[i])[_qp]);
    f0 += sum_etaj * Utility::pow<2>((*_vals[i])[_qp]) * _gamma[_qp];
  }
  return _dmudvar[_qp] * f0;
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

template class ACBarrierFunctionTempl<false>;
template class ACBarrierFunctionTempl<true>;
