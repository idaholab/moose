//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACGrGrMulti.h"

registerMooseObject("PhaseFieldApp", ACGrGrMulti);
registerMooseObject("PhaseFieldApp", ADACGrGrMulti);

template <bool is_ad>
InputParameters
ACGrGrMultiTempl<is_ad>::validParams()
{
  InputParameters params = ACGrGrMultiBase<is_ad>::validParams();
  params.addClassDescription("Multi-phase poly-crystalline Allen-Cahn Kernel");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "gamma_names",
      "List of gamma material property names for each other order parameter. Place "
      "in same order as order parameters (v)!");
  return params;
}

template <bool is_ad>
ACGrGrMultiTempl<is_ad>::ACGrGrMultiTempl(const InputParameters & parameters)
  : ACGrGrMultiBase<is_ad>(parameters),
    _gamma_names(this->template getParam<std::vector<MaterialPropertyName>>("gamma_names")),
    _num_j(_gamma_names.size()),
    _prop_gammas(_num_j)
{
  // check passed in parameter vectors
  if (_num_j != this->coupledComponents("v"))
    this->paramError(
        "gamma_names",
        "Need to pass in as many gamma_names as coupled variables in v in ACGrGrMulti");

  for (unsigned int n = 0; n < _num_j; ++n)
    _prop_gammas[n] = &this->template getGenericMaterialProperty<Real, is_ad>(_gamma_names[n]);
}

ACGrGrMulti::ACGrGrMulti(const InputParameters & parameters)
  : ACGrGrMultiTempl<false>(parameters),
    _uname(this->template getParam<NonlinearVariableName>("variable")),
    _dmudu(this->template getMaterialPropertyDerivative<Real>("mu", _uname)),
    _vname(this->template getParam<std::vector<VariableName>>("v")),
    _dmudEtaj(_num_j)
{
  for (unsigned int n = 0; n < _num_j; ++n)
    _dmudEtaj[n] = &this->template getMaterialPropertyDerivative<Real>("mu", _vname[n]);
}

Real
ACGrGrMulti::computeDFDOP(PFFunctionType type)
{
  // Sum all other order parameters
  Real SumGammaEtaj = 0.0;
  for (const auto i : make_range(_op_num))
    SumGammaEtaj += (*_prop_gammas[i])[_qp] * (*_vals[i])[_qp] * (*_vals[i])[_qp];

  // Calculate either the residual or Jacobian of the grain growth free energy
  switch (type)
  {
    case Residual:
    {
      return _mu[_qp] * computedF0du();
    }

    case Jacobian:
    {
      Real d2f0du2 = 3.0 * _u[_qp] * _u[_qp] - 1.0 + 2.0 * SumGammaEtaj;
      return _phi[_j][_qp] * (_mu[_qp] * d2f0du2 + _dmudu[_qp] * computedF0du());
    }

    default:
      mooseError("Invalid type passed in");
  }
}

ADReal
ADACGrGrMulti::computeDFDOP()
{
  // Sum all other order parameters
  ADReal SumGammaEtaj = 0.0;
  for (const auto i : make_range(_op_num))
    SumGammaEtaj += (*_prop_gammas[i])[_qp] * (*_vals[i])[_qp] * (*_vals[i])[_qp];

  return _mu[_qp] * computedF0du();
}

Real
ACGrGrMulti::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i = 0; i < _op_num; ++i)
    if (jvar == _vals_var[i])
    {
      // Derivative of SumGammaEtaj
      const Real dSumGammaEtaj = 2.0 * (*_prop_gammas[i])[_qp] * (*_vals[i])[_qp];
      const Real dDFDOP = _mu[_qp] * 2.0 * _u[_qp] * dSumGammaEtaj;

      return _L[_qp] * _test[_i][_qp] * _phi[_j][_qp] *
             (dDFDOP + (*_dmudEtaj[i])[_qp] * computedF0du());
    }

  return 0.0;
}

template <bool is_ad>
GenericReal<is_ad>
ACGrGrMultiTempl<is_ad>::computedF0du()
{
  GenericReal<is_ad> SumGammaEtaj = 0.0;
  for (const auto i : make_range(_op_num))
    SumGammaEtaj += (*_prop_gammas[i])[_qp] * (*_vals[i])[_qp] * (*_vals[i])[_qp];

  return _u[_qp] * _u[_qp] * _u[_qp] - _u[_qp] + 2.0 * _u[_qp] * SumGammaEtaj;
}

template class ACGrGrMultiTempl<false>;
template class ACGrGrMultiTempl<true>;
