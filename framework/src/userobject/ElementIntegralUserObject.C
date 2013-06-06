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

#include "ElementIntegralUserObject.h"

template<>
InputParameters validParams<ElementIntegralUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  return params;
}

ElementIntegralUserObject::ElementIntegralUserObject(const std::string & name, InputParameters parameters) :
    ElementUserObject(name, parameters),
    _qp(0),
    _integral_value(0)
{}

void
ElementIntegralUserObject::initialize()
{
  _integral_value = 0;
}

void
ElementIntegralUserObject::execute()
{
  _integral_value += computeIntegral();
}

Real
ElementIntegralUserObject::getValue()
{
  gatherSum(_integral_value);
  return _integral_value;
}

void
ElementIntegralUserObject::threadJoin(const UserObject & y)
{
  const ElementIntegralUserObject & pps = static_cast<const ElementIntegralUserObject &>(y);
  _integral_value += pps._integral_value;
}

Real
ElementIntegralUserObject::computeIntegral()
{
  Real sum = 0;

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    sum += _JxW[_qp]*_coord[_qp]*computeQpIntegral();
  return sum;
}
