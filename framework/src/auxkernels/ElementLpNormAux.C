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
#include "ElementLpNormAux.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ElementLpNormAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRangeCheckedParam<Real>("p", 2.0, "p>=1", "The exponent used in the norm.");
  params.addRequiredCoupledVar("coupled_variable", "The variable to compute the norm of.");
  return params;
}

ElementLpNormAux::ElementLpNormAux(const InputParameters & parameters)
  : AuxKernel(parameters), _p(getParam<Real>("p")), _coupled_var(coupledValue("coupled_variable"))
{
}

void
ElementLpNormAux::compute()
{
  precalculateValue();

  if (isNodal())
    mooseError("ElementLpNormAux only makes sense as an Elemental AuxVariable.");

  // Sum up the squared-error values by calling computeValue(), then
  // return the sqrt of the result.
  Real summed_value = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    Real val = computeValue();
    summed_value += _JxW[_qp] * _coord[_qp] * std::pow(std::abs(val), _p);
  }

  _var.setNodalValue(std::pow(summed_value, 1. / _p));
}

Real
ElementLpNormAux::computeValue()
{
  return _coupled_var[_qp];
}
