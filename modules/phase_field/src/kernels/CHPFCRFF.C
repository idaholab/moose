//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CHPFCRFF.h"
#include "MathUtils.h"

registerMooseObject("PhaseFieldApp", CHPFCRFF);

InputParameters
CHPFCRFF::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Cahn-Hilliard residual for the RFF form of the phase field crystal model");
  params.addRequiredCoupledVar("v", "Array of names of the real parts of the L variables");
  MooseEnum log_options("tolerance cancelation expansion nothing");
  params.addRequiredParam<MooseEnum>(
      "log_approach", log_options, "Which approach will be used to handle the natural log");
  params.addParam<Real>("tol", 1.0e-9, "Tolerance used when the tolerance approach is chosen");
  params.addParam<Real>(
      "n_exp_terms", 4, "Number of terms used in the Taylor expansion of the natural log term");
  params.addParam<MaterialPropertyName>("mob_name", "M", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("Dmob_name", "DM", "The D mobility used with the kernel");
  params.addParam<bool>("has_MJac", false, "Jacobian information for the mobility is defined");
  params.addParam<Real>("a", 1.0, "Constants on Taylor Series");
  params.addParam<Real>("b", 1.0, "Constants on Taylor Series");
  params.addParam<Real>("c", 1.0, "Constants on Taylor Series");
  return params;
}

CHPFCRFF::CHPFCRFF(const InputParameters & parameters)
  : Kernel(parameters),
    _M(getMaterialProperty<Real>("mob_name")),
    _has_MJac(getParam<bool>("has_MJac")),
    _DM(_has_MJac ? &getMaterialProperty<Real>("Dmob_name") : NULL),
    _log_approach(getParam<MooseEnum>("log_approach")),
    _tol(getParam<Real>("tol")),
    _num_L(coupledComponents("v")),
    _vals_var(_num_L),
    _grad_vals(_num_L),
    _n_exp_terms(getParam<Real>("n_exp_terms")),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),
    _c(getParam<Real>("c"))
{
  // Loop through grains and load coupled gradients into the arrays
  for (unsigned int i = 0; i < _num_L; ++i)
  {
    _vals_var[i] = coupled("v", i);
    _grad_vals[i] = &coupledGradient("v", i);
  }
}

Real
CHPFCRFF::computeQpResidual()
{
  Real c = _u[_qp];
  RealGradient grad_c = _grad_u[_qp];
  RealGradient sum_grad_L;

  for (unsigned int i = 0; i < _num_L; ++i)
    sum_grad_L += (*_grad_vals[i])[_qp] * 0.5;

  Real frac = 0.0;
  Real ln_expansion = 0.0;

  switch (_log_approach)
  {
    case 0: // approach using tolerance
      if (1.0 + c < _tol)
        frac = 1.0 / _tol;
      else
        frac = 1.0 / (1.0 + c);
      break;

    case 2:
      for (unsigned int i = 2; i < (_n_exp_terms + 2.0); ++i)
      {
        // Apply Coefficents to Taylor Series defined in input file
        Real temp_coeff;
        if (i == 2)
          temp_coeff = _c;
        else if (i == 3)
          temp_coeff = _a;
        else if (i == 4)
          temp_coeff = _b;
        else
          temp_coeff = 1.0;

        ln_expansion += temp_coeff * std::pow(-1.0, Real(i)) * std::pow(_u[_qp], Real(i) - 2.0);
      }
      break;
  }

  RealGradient GradDFDCons;

  switch (_log_approach)
  {
    case 0: // approach using tolerance
      GradDFDCons = grad_c * frac - sum_grad_L;
      break;

    case 1: // approach using cancelation from the mobility
      GradDFDCons = grad_c - (1.0 + c) * sum_grad_L;
      break;

    case 2: // appraoch using substitution
      GradDFDCons = ln_expansion * grad_c - sum_grad_L;
      break;

    case 3: // Just using the log
      GradDFDCons = grad_c / (1.0 + c) - sum_grad_L;
      break;
  }

  Real residual = _M[_qp] * GradDFDCons * _grad_test[_i][_qp];
  return residual;
}

Real
CHPFCRFF::computeQpJacobian()
{
  Real c = _u[_qp];
  RealGradient grad_c = _grad_u[_qp];
  RealGradient sum_grad_L;

  for (unsigned int i = 0; i < _num_L; ++i)
    sum_grad_L += (*_grad_vals[i])[_qp] * 0.5;

  Real frac = 0.0;
  Real dfrac = 0.0;
  Real ln_expansion = 0.0;

  switch (_log_approach)
  {
    case 0: // approach using tolerance
      if (1.0 + c < _tol)
      {
        frac = 1.0 / _tol;
        dfrac = -1.0 / (_tol * _tol);
      }
      else
      {
        frac = 1.0 / (1.0 + c);
        dfrac = -1.0 / ((1.0 + c) * (1.0 + c));
      }
      break;

    case 2:
      for (unsigned int i = 2; i < (_n_exp_terms + 2.0); ++i)
      {
        // Apply Coefficents to Taylor Series defined in input file
        Real temp_coeff;
        if (i == 2)
          temp_coeff = _c;
        else if (i == 3)
          temp_coeff = _a;
        else if (i == 4)
          temp_coeff = _b;
        else
          temp_coeff = 1.0;

        ln_expansion += temp_coeff * std::pow(-1.0, Real(i)) * std::pow(_u[_qp], Real(i) - 2.0);
      }
      break;
  }

  RealGradient dGradDFDConsdC;
  Real Dln_expansion = 0.0;

  switch (_log_approach)
  {
    case 0: // approach using tolerance
      dGradDFDConsdC = _grad_phi[_j][_qp] * frac + _phi[_j][_qp] * grad_c * dfrac;
      break;

    case 1: // approach using cancelation from the mobility
      dGradDFDConsdC = _grad_phi[_j][_qp] - _phi[_j][_qp] * sum_grad_L;
      break;

    case 2: // appraoch using substitution
      for (unsigned int i = 2; i < (_n_exp_terms + 2.0); ++i)
      {
        Real temp_coeff;
        if (i == 2)
          temp_coeff = _c;
        else if (i == 3)
          temp_coeff = _a;
        else if (i == 4)
          temp_coeff = _b;
        else
          temp_coeff = 1.0;

        Dln_expansion += temp_coeff * std::pow(static_cast<Real>(-1.0), static_cast<Real>(i)) *
                         (static_cast<Real>(i) - 2.0) *
                         std::pow(_u[_qp], static_cast<Real>(i) - 3.0);
      }

      dGradDFDConsdC = ln_expansion * _grad_phi[_j][_qp] + _phi[_j][_qp] * Dln_expansion * grad_c;
      break;

    case 3: // Nothing special
      dGradDFDConsdC =
          _grad_phi[_j][_qp] / (1.0 + c) - grad_c / ((1.0 + c) * (1.0 + c)) * _phi[_j][_qp];
      break;
  }

  return _M[_qp] * dGradDFDConsdC * _grad_test[_i][_qp];
}

Real
CHPFCRFF::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real c = _u[_qp];

  for (unsigned int i = 0; i < _num_L; ++i)
    if (jvar == _vals_var[i])
    {

      RealGradient dsum_grad_L = _grad_phi[_j][_qp] * 0.5;
      RealGradient dGradDFDConsdL;
      switch (_log_approach)
      {
        case 0: // approach using tolerance
          dGradDFDConsdL = -dsum_grad_L;
          break;

        case 1: // approach using cancelation from the mobility
          dGradDFDConsdL = -(1.0 + c) * dsum_grad_L;
          break;

        case 2: // appraoch using substitution
          dGradDFDConsdL = -dsum_grad_L;
          break;

        case 3: // nothing special
          dGradDFDConsdL = -dsum_grad_L;
          break;
      }

      return _M[_qp] * dGradDFDConsdL * _grad_test[_i][_qp];
    }

  return 0.0;
}
