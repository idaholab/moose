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

#include "ElementL2Norm.h"

template<>
InputParameters validParams<ElementL2Norm>()
{
  InputParameters params = validParams<ElementIntegral>();
  return params;
}

ElementL2Norm::ElementL2Norm(const std::string & name, InputParameters parameters) :
    ElementIntegral(name, parameters)
{
}

Real
ElementL2Norm::getValue()
{
  return std::sqrt(ElementIntegral::getValue());
}

Real
ElementL2Norm::computeQpIntegral()
{
  Real val = _u[_qp];
  return val*val;
}
