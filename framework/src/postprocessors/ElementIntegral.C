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

#include "ElementIntegral.h"

template<>
InputParameters validParams<ElementIntegral>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  return params;
}

ElementIntegral::ElementIntegral(const std::string & name, InputParameters parameters) :
    ElementPostprocessor(name, parameters),
    _q_point(_subproblem.points(_tid)),
    _qrule(_subproblem.qRule(_tid)),
    _JxW(_subproblem.JxW(_tid)),
    _coord(_subproblem.coords(_tid)),
    _u(_var.sln()),
    _grad_u(_var.gradSln()),
    _integral_value(0)
{}

void
ElementIntegral::initialize()
{
  _integral_value = 0;
}

void
ElementIntegral::execute()
{
  _integral_value += computeIntegral();
}

Real
ElementIntegral::getValue()
{
  gatherSum(_integral_value);
  return _integral_value;
}

void
ElementIntegral::threadJoin(const UserObject & y)
{
  const ElementIntegral & pps = dynamic_cast<const ElementIntegral &>(y);
  _integral_value += pps._integral_value;
}

Real
ElementIntegral::computeQpIntegral()
{
  return _u[_qp];
}

Real
ElementIntegral::computeIntegral()
{
  Real sum = 0;

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    sum += _JxW[_qp]*_coord[_qp]*computeQpIntegral();
  return sum;
}
