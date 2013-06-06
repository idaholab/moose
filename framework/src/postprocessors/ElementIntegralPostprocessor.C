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

template<>
InputParameters validParams<ElementIntegralPostprocessor>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  return params;
}

ElementIntegralPostprocessor::ElementIntegralPostprocessor(const std::string & name, InputParameters parameters) :
    ElementPostprocessor(name, parameters),
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
  return _integral_value;
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
