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
#include "RealFunctionControl.h"
#include "Function.h"

template<>
InputParameters validParams<RealFunctionControl>()
{
  InputParameters params = validParams<Control>();

  params.addRequiredParam<FunctionName>("function", "The function to use for controlling the specified parameter.");
  params.addRequiredParam<std::string>("parameter", "The input parameter(s) to control. Specify a single parameter name and all parameters in all objects matching the name will be updated");
  params.addParam<unsigned int>("num_parameters", "The expected number of parameters, if nothing is provide no limit is given to the number of parameters.");
  return params;
}

RealFunctionControl::RealFunctionControl(const InputParameters & parameters) :
    Control(parameters),
    _function(getFunction("function")),
    _parameters(isParamValid("num_parameters") ? getControlParam<Real>("parameter", getParam<unsigned int>("num_parameters")) : getControlParam<Real>("parameter"))
{
}

void
RealFunctionControl::execute()
{
  Real value = _function.value(_t, Point());
  _parameters.set(value);
}
