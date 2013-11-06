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

#include "MooseParsedGradFunction.h"

template<>
InputParameters validParams<MooseParsedGradFunction>()
{
  InputParameters params = validParams<MooseParsedFunctionBase>();
  params.addParam<std::string>("value", "0", "User defined function.");
  params.addParam<std::string>("grad_x", "0", "Partial with respect to x.");
  params.addParam<std::string>("grad_y", "0", "Partial with respect to y.");
  params.addParam<std::string>("grad_z", "0", "Partial with respect to z.");
  return params;
}

MooseParsedGradFunction::MooseParsedGradFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
    MooseParsedFunctionBase(name, parameters),
    _value(verifyFunction(getParam<std::string>("value"))),
    _grad_value(verifyFunction(std::string("{") + getParam<std::string>("grad_x") + "}{" +
                                 getParam<std::string>("grad_y") + "}{" +
                                 getParam<std::string>("grad_z") + "}")),
    _function_ptr(NULL),
    _grad_function_ptr(NULL)
{
}

MooseParsedGradFunction::~MooseParsedGradFunction()
{
  // Clean up the parsed function object
  delete _function_ptr;
  delete _grad_function_ptr;
}

Real
MooseParsedGradFunction::value(Real t, const Point & p)
{
  // Return a scalar value
  return _function_ptr->evaluate<Real>(t, p);
}

RealGradient
MooseParsedGradFunction::gradient(Real t, const Point & p)
{
  // Return gradient (RealGradient = RealVectorValue)
  return _grad_function_ptr->evaluate<RealVectorValue>(t, p);
}

RealVectorValue
MooseParsedGradFunction::vectorValue(Real /*t*/, const Point & /*p*/)
{
  mooseError("The vectorValue method is not defined in ParsedGradFunction");
}

void
MooseParsedGradFunction::initialSetup()
{
  if (_function_ptr == NULL)
    _function_ptr = new MooseParsedFunctionWrapper(_pfb_feproblem, _value, _vars, _vals);

  if (_grad_function_ptr == NULL)
    _grad_function_ptr = new MooseParsedFunctionWrapper(_pfb_feproblem, _grad_value, _vars, _vals);
}
