/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "ElementH1ErrorFunctionAux.h"
#include "Function.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ElementH1ErrorFunctionAux>()
{
  InputParameters params = validParams<ElementL2ErrorFunctionAux>();
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
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    val += std::pow(std::abs(graddiff(i)), _p);

  return val;
}
