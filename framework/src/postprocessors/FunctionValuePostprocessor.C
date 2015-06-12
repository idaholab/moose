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
#include "FunctionValuePostprocessor.h"
#include "Function.h"

template<>
InputParameters validParams<FunctionValuePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<FunctionName>("function", "The function which supplies the postprocessor value.");

  return params;
}

FunctionValuePostprocessor::FunctionValuePostprocessor(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _function(getFunction("function"))
{
}

FunctionValuePostprocessor::~FunctionValuePostprocessor()
{
}

void
FunctionValuePostprocessor::initialize()
{
}

void
FunctionValuePostprocessor::execute()
{
}

PostprocessorValue
FunctionValuePostprocessor::getValue()
{
  return _function.value(_t, 0);         //Pass 0 instead of Point(0,0,0) because postprocessors return a single scalar value.
}

void
FunctionValuePostprocessor::threadJoin(const UserObject & /*uo*/)
{
  // nothing to do here, general PPS do not run threaded
}
