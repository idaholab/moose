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

#include "NodalL2Error.h"
#include "Function.h"

template<>
InputParameters validParams<NodalL2Error>()
{
  InputParameters params = validParams<NodalVariablePostprocessor>();
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");

  return params;
}

NodalL2Error::NodalL2Error(const std::string & name, InputParameters parameters) :
    NodalVariablePostprocessor(name, parameters),
    FunctionInterface(parameters),
    _func(getFunction("function"))
{
}

NodalL2Error::~NodalL2Error()
{
}

void
NodalL2Error::initialize()
{
  _integral_value = 0.;
}

void
NodalL2Error::execute()
{
  Real diff = _u[0] - _func.value(_t, *_current_node);
  _integral_value += diff * diff;
}

Real
NodalL2Error::getValue()
{
  gatherSum(_integral_value);
  return std::sqrt(_integral_value);
}

void
NodalL2Error::threadJoin(const UserObject & y)
{
  const NodalL2Error & pps = dynamic_cast<const NodalL2Error &>(y);
  _integral_value += pps._integral_value;
}
