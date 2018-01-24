/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PFCRFFEnergyDensity.h"
#include "libmesh/utility.h"

template <>
InputParameters
validParams<PFCRFFEnergyDensity>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("v", "Array of coupled variables");
  params.addParam<Real>("a", 1.0, "Modified Coefficent in Taylor Series Expanstion");
  params.addParam<Real>("b", 1.0, "Modified Coefficent in Taylor Series Expanstion");
  params.addParam<Real>("c", 1.0, "Modified Coefficent in Taylor Series Expanstion");
  params.addParam<unsigned int>(
      "num_exp_terms", 4, "This is the number of terms to use in the taylor series expansion");
  MooseEnum log_options("tolerance cancelation expansion nothing");
  params.addRequiredParam<MooseEnum>(
      "log_approach", log_options, "Which approach will be used to handle the natural log");
  params.addParam<Real>("tol", 1.0e-9, "Tolerance used when the tolerance approach is chosen");
  return params;
}

PFCRFFEnergyDensity::PFCRFFEnergyDensity(const InputParameters & parameters)
  : AuxKernel(parameters),
    _order(coupledComponents("v")),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),
    _c(getParam<Real>("c")),
    _num_exp_terms(getParam<unsigned int>("num_exp_terms")),
    _log_approach(getParam<MooseEnum>("log_approach")),
    _tol(getParam<Real>("tol"))
{
  _vals.resize(_order);
  for (unsigned int i = 0; i < _order; ++i)
    _vals[i] = &coupledValue("v", i);
}

Real
PFCRFFEnergyDensity::computeValue()
{
  Real val = 0.0;
  switch (_log_approach)
  {
    case 0: // approach using tolerence
      if (1.0 + (*_vals[0])[_qp] < _tol)
        val += ((1.0 + _tol) * std::log(1 + _tol)) - _tol;
      else
        val += ((1.0 + (*_vals[0])[_qp]) * std::log(1 + (*_vals[0])[_qp])) - (*_vals[0])[_qp];
      break;

    case 1: // approach using cancellation
      val += ((1.0 + (*_vals[0])[_qp]) * std::log(1.0 + (*_vals[0])[_qp])) - (*_vals[0])[_qp];
      break;

    case 2: // approach using Taylor Series Expansion
      Real coef = 1.0;

      for (unsigned int i = 2; i < (2 + _num_exp_terms); i++)
      {
        if (i == 2)
          coef = _c;
        else if (i == 3)
          coef = _a;
        else if (i == 4)
          coef = _b;
        else
          coef = 1.0;

        val +=
            coef * (std::pow(-1.0, Real(i)) / (i * (i - 1))) * std::pow((*_vals[0])[_qp], Real(i));
      }
      break;
  }

  // Loop Through Variables
  Real sumL = 0.0;
  for (unsigned int i = 1; i < _order; ++i)
    sumL += (*_vals[i])[_qp] * 0.5;

  val -= ((*_vals[0])[_qp] * sumL);

  return val;
}
