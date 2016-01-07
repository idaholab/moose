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

#include "ElementIntegralPostprocessor.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<ElementIntegralPostprocessor>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  params.addParam<Real>("scaling", 1.0, "A simple scaling factor applied to the postprocessor");
  return params;
}

ElementIntegralPostprocessor::ElementIntegralPostprocessor(const InputParameters & parameters) :
    ElementPostprocessor(parameters),
    _scaling(getParam<Real>("scaling")),
    _qp(0),
    _integral_value(0)
{}

void
ElementIntegralPostprocessor::initialize()
{
  _integral_value = 0;
}

void
ElementIntegralPostprocessor::execute()
{
  _integral_value += computeIntegral();
}

Real
ElementIntegralPostprocessor::getValue()
{
  gatherSum(_integral_value);
  return _scaling * _integral_value;
}

void
ElementIntegralPostprocessor::threadJoin(const UserObject & y)
{
  const ElementIntegralPostprocessor & pps = static_cast<const ElementIntegralPostprocessor &>(y);
  _integral_value += pps._integral_value;
}

Real
ElementIntegralPostprocessor::computeIntegral()
{
  Real sum = 0;

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    sum += _JxW[_qp]*_coord[_qp]*computeQpIntegral();
  return sum;
}

