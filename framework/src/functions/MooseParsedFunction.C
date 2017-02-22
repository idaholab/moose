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

#include "MooseError.h"
#include "MooseParsedFunction.h"
#include "MooseParsedFunctionWrapper.h"

template<>
InputParameters validParams<MooseParsedFunction>()
{
  InputParameters params = validParams<Function>();
  params += validParams<MooseParsedFunctionBase>();
  params.addRequiredParam<std::string>("value", "The user defined function.");
  return params;
}

MooseParsedFunction::MooseParsedFunction(const InputParameters & parameters) :
    Function(parameters),
    MooseParsedFunctionBase(parameters),
    _value(verifyFunction(getParam<std::string>("value")))
{
}

Real
MooseParsedFunction::value(Real t, const Point & p)
{
  return _function_ptr->evaluate<Real>(t, p);
}

RealGradient
MooseParsedFunction::gradient(Real t, const Point & p)
{
  return _function_ptr->evaluateGradient(t, p);
}

Real
MooseParsedFunction::timeDerivative(Real t, const Point & p)
{
  return _function_ptr->evaluateDot(t, p);
}

RealVectorValue
MooseParsedFunction::vectorValue(Real /*t*/, const Point & /*p*/)
{
  mooseError("The vectorValue method is not defined in ParsedFunction");
}

void
MooseParsedFunction::initialSetup()
{
  if (!_function_ptr)
  {
    THREAD_ID tid = 0;
    if (isParamValid("_tid"))
      tid = getParam<THREAD_ID>("_tid");

    _function_ptr = libmesh_make_unique<MooseParsedFunctionWrapper>(_pfb_feproblem, _value, _vars, _vals, tid);
  }
}
