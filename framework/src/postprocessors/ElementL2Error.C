/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ElementL2Error.h"
#include "Function.h"

template<>
InputParameters validParams<ElementL2Error>()
{
  InputParameters params = validParams<ElementIntegral>();
  params.addRequiredParam<std::string>("function", "The analytic solution to compare against");
  return params;
}

ElementL2Error::ElementL2Error(std::string name,
                             MooseSystem & moose_system,
                             InputParameters parameters):
  ElementIntegral(name, moose_system, parameters),
  _func(getFunction("function"))
{
}

Real
ElementL2Error::getValue()
{
  return std::sqrt(ElementIntegral::getValue());
}

Real
ElementL2Error::computeQpIntegral()
{
  Real diff = _u[_qp]-_func(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
  return diff*diff;
}
