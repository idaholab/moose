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

#include "ElementAverageTimeDerivative.h"

template<>
InputParameters validParams<ElementAverageTimeDerivative>()
{
  InputParameters params = validParams<ElementAverageValue>();
  return params;
}



ElementAverageTimeDerivative::ElementAverageTimeDerivative(const std::string & name, InputParameters parameters) :
    ElementAverageValue(name, parameters)
{}



Real
ElementAverageTimeDerivative::computeQpIntegral()
{
  return _u_dot[_qp];
}
