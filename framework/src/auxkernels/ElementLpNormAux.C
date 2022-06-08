//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ElementLpNormAux.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", ElementLpNormAux);

InputParameters
ElementLpNormAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Compute an elemental field variable (single value per element) equal "
                             "to the Lp-norm of a coupled Variable.");
  params.addRangeCheckedParam<Real>("p", 2.0, "p>=1", "The exponent used in the norm.");
  params.addRequiredCoupledVar("coupled_variable", "The variable to compute the norm of.");
  return params;
}

ElementLpNormAux::ElementLpNormAux(const InputParameters & parameters)
  : AuxKernel(parameters), _p(getParam<Real>("p")), _coupled_var(coupledValue("coupled_variable"))
{
  if (mooseVariableBase()->feType() != FEType(CONSTANT, MONOMIAL))
    paramError("variable", "Must be of type CONSTANT MONOMIAL");
}

void
ElementLpNormAux::compute()
{
  precalculateValue();

  // Sum up the squared-error values by calling computeValue(), then
  // return the sqrt of the result.
  Real summed_value = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    Real val = computeValue();
    summed_value += _JxW[_qp] * _coord[_qp] * std::pow(std::abs(val), _p);
  }

  _var.setDofValue(std::pow(summed_value, 1. / _p), 0);
}

Real
ElementLpNormAux::computeValue()
{
  return _coupled_var[_qp];
}
