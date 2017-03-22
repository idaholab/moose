/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACGrGrMulti.h"

template <>
InputParameters
validParams<ACGrGrMulti>()
{
  InputParameters params = validParams<ACGrGrBase>();
  params.addClassDescription("Multi-phase poly-crystaline Allen-Cahn Kernel");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "gamma_names",
      "List of gamma material property names for each other order parameter. Place "
      "in same order as order parameters (v)!");
  return params;
}

ACGrGrMulti::ACGrGrMulti(const InputParameters & parameters)
  : ACGrGrBase(parameters),
    _gamma_names(getParam<std::vector<MaterialPropertyName>>("gamma_names")),
    _num_j(_gamma_names.size()),
    _prop_gammas(_num_j)
{
  // check passed in parameter vectors
  if (_num_j != coupledComponents("v"))
    mooseError("Need to pass in as many gamma_names as coupled variables in v in ACGrGrMulti",
               name());

  for (unsigned int n = 0; n < _num_j; ++n)
    _prop_gammas[n] = &getMaterialPropertyByName<Real>(_gamma_names[n]);
}

Real
ACGrGrMulti::computeDFDOP(PFFunctionType type)
{
  // Sum all other order parameters
  Real SumGammaEtaj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumGammaEtaj += (*_prop_gammas[i])[_qp] * (*_vals[i])[_qp] * (*_vals[i])[_qp];

  // Calculate either the residual or Jacobian of the grain growth free energy
  switch (type)
  {
    case Residual:
    {
      const Real tgrad_correction =
          _grad_T ? _tgrad_corr_mult[_qp] * _grad_u[_qp] * (*_grad_T)[_qp] : 0.0;
      return _mu[_qp] * (_u[_qp] * _u[_qp] * _u[_qp] - _u[_qp] + 2.0 * _u[_qp] * SumGammaEtaj) +
             tgrad_correction;
    }

    case Jacobian:
    {
      const Real tgrad_correction =
          _grad_T ? _tgrad_corr_mult[_qp] * _grad_phi[_j][_qp] * (*_grad_T)[_qp] : 0.0;
      return _mu[_qp] * (_phi[_j][_qp] * (3.0 * _u[_qp] * _u[_qp] - 1.0 + 2.0 * SumGammaEtaj)) +
             tgrad_correction;
    }

    default:
      mooseError("Invalid type passed in");
  }
}

Real
ACGrGrMulti::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i = 0; i < _op_num; ++i)
    if (jvar == _vals_var[i])
    {
      // Derivative of SumGammaEtaj
      const Real dSumGammaEtaj = 2.0 * (*_prop_gammas[i])[_qp] * (*_vals[i])[_qp] * _phi[_j][_qp];
      const Real dDFDOP = _mu[_qp] * 2.0 * _u[_qp] * dSumGammaEtaj;

      return _L[_qp] * _test[_i][_qp] * dDFDOP;
    }

  return 0.0;
}
