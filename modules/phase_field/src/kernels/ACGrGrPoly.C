/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACGrGrPoly.h"

template<>
InputParameters validParams<ACGrGrPoly>()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription("Grain-Boundary model poly-crystaline interface Allen-Cahn Kernel");
  params.addRequiredCoupledVar("v", "Array of coupled variable names");
  params.addCoupledVar("T", "temperature");
  return params;
}

ACGrGrPoly::ACGrGrPoly(const InputParameters & parameters) :
    ACBulk<Real>(parameters),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _vals_var(_op_num),
    _mu(getMaterialProperty<Real>("mu")),
    _gamma(getMaterialProperty<Real>("gamma_asymm")),
    _tgrad_corr_mult(getMaterialProperty<Real>("tgrad_corr_mult")),
    _grad_T(isCoupled("T") ? &coupledGradient("T") : NULL)
{
  // Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _vals[i] = &coupledValue("v", i);
    _vals_var[i] = coupled("v", i);
  }
}

Real
ACGrGrPoly::computeDFDOP(PFFunctionType type)
{
  // Sum all other order parameters
  Real SumEtaj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumEtaj += (*_vals[i])[_qp] * (*_vals[i])[_qp];

  // Calculate either the residual or Jacobian of the grain growth free energy
  switch (type)
  {
    case Residual:
    {
      const Real tgrad_correction = _grad_T ? _tgrad_corr_mult[_qp] * _grad_u[_qp] * (*_grad_T)[_qp] : 0.0;
      return   _mu[_qp] * (_u[_qp]*_u[_qp]*_u[_qp] - _u[_qp] + 2.0 * _gamma[_qp] * _u[_qp] * SumEtaj)
             + tgrad_correction;
    }

    case Jacobian:
    {
      const Real tgrad_correction = _grad_T ? _tgrad_corr_mult[_qp] * _grad_phi[_j][_qp] * (*_grad_T)[_qp] : 0.0;
      return   _mu[_qp] * (_phi[_j][_qp] * (3.0 * _u[_qp] * _u[_qp] - 1.0 + 2.0 * _gamma[_qp] * SumEtaj))
             + tgrad_correction;
    }

    default:
      mooseError("Invalid type passed in");
  }
}

Real
ACGrGrPoly::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i = 0; i < _op_num; ++i)
    if (jvar == _vals_var[i])
    {
      // Derivative of SumEtaj
      const Real dSumEtaj = 2.0 * (*_vals[i])[_qp] * _phi[_j][_qp];
      const Real dDFDOP = _mu[_qp] * 2.0 * _gamma[_qp] * _u[_qp] * dSumEtaj;

      return _L[_qp] * _test[_i][_qp] * dDFDOP;
    }

  return 0.0;
}
