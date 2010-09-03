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

#include "UserForcingFunction.h"
#include "Function.h"

template<>
InputParameters validParams<UserForcingFunction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("function", "The forcing function");
  return params;
}

UserForcingFunction::UserForcingFunction(std::string name,
                       MooseSystem &sys,
                       InputParameters parameters)
  :Kernel(name, sys, parameters),
  _func(getFunction("function"))
{
}

Real
UserForcingFunction::f()
{
  return _func(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
}

Real
UserForcingFunction::computeQpResidual()
{
  return -_test[_i][_qp] * f();
}
