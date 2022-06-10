//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ElementH1ErrorFunctionAux.h"
#include "Function.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", ElementH1ErrorFunctionAux);

InputParameters
ElementH1ErrorFunctionAux::validParams()
{
  InputParameters params = ElementL2ErrorFunctionAux::validParams();
  params.addClassDescription(
      "Computes the H1 or W^{1,p} error between an exact function and a coupled variable.");

  return params;
}

ElementH1ErrorFunctionAux::ElementH1ErrorFunctionAux(const InputParameters & parameters)
  : ElementL2ErrorFunctionAux(parameters), _grad_coupled_var(coupledGradient("coupled_variable"))
{
}

void
ElementH1ErrorFunctionAux::compute()
{
  precalculateValue();

  if (isNodal())
    mooseError("ElementH1ErrorFunctionAux only makes sense as an Elemental AuxVariable.");

  Real summed_value = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    Real val = computeValue(); // already raised to the p, see below.
    summed_value += _JxW[_qp] * _coord[_qp] * val;
  }

  _var.setNodalValue(std::pow(summed_value, 1. / _p));
}

Real
ElementH1ErrorFunctionAux::computeValue()
{
  RealGradient graddiff = _func.gradient(_t, _q_point[_qp]) - _grad_coupled_var[_qp];
  Real funcdiff = _func.value(_t, _q_point[_qp]) - _coupled_var[_qp];

  // Raise the absolute function value difference to the pth power
  Real val = std::pow(std::abs(funcdiff), _p);

  // Add all of the absolute gradient component differences to the pth power
  for (const auto i : make_range(Moose::dim))
    val += std::pow(std::abs(graddiff(i)), _p);

  return val;
}
