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

#include "TestSetupPostprocessorDataActionFunction.h"

template<>
InputParameters validParams<TestSetupPostprocessorDataActionFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<PostprocessorName>("postprocessor", "A postprocessor to test against");
  return params;
}

TestSetupPostprocessorDataActionFunction::TestSetupPostprocessorDataActionFunction(const std::string & name, InputParameters parameters) :
  Function(name, parameters)
{
  if (hasPostprocessor("postprocessor"))
    mooseError("TestSetupPostprocessorDataActionFunction pass");
  else
    mooseError("TestSetupPostprocessorDataActionFunction fail");

}

TestSetupPostprocessorDataActionFunction::~TestSetupPostprocessorDataActionFunction()
{
}
