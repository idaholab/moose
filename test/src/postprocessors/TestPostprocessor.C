//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestPostprocessor.h"
#include "MooseTestAppTypes.h"
#include "Conversion.h"

registerMooseObject("MooseTestApp", TestPostprocessor);

InputParameters
TestPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum test_type("grow use_older_value report_old custom_execute_on");
  params.addRequiredParam<MooseEnum>("test_type", test_type, "The type of test to perform");
  params.addParam<PostprocessorName>(
      "report_name", 0, "The name of the postprocessor value to report");
  ExecFlagEnum & exec = params.set<ExecFlagEnum>("execute_on");
  exec.addAvailableFlags(EXEC_JUST_GO);
  params.setDocString("execute_on", exec.getDocString());
  return params;
}

TestPostprocessor::TestPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _test_type(getParam<MooseEnum>("test_type")),
    _old_val(getPostprocessorValueOldByName(name())),
    _older_val(getPostprocessorValueOlderByName(name())),
    _report_old(getPostprocessorValueOld("report_name")),
    _execute_count(declareRestartableData<unsigned int>("execute_count"))
{
  if (_test_type == "report_old" && !isParamValid("report_name"))
    mooseError("Must set 'report_name' parameter when using the 'report_old' test type.");

  if (!_app.isRecovering())
    _execute_count = 0;
}

void
TestPostprocessor::customSetup(const ExecFlagType & exec_type)
{
  if (exec_type == EXEC_JUST_GO)
    _console << "Flag Name: " << EXEC_JUST_GO << std::endl;
}

Real
TestPostprocessor::getValue()
{
  if (_test_type == "grow")
  {
    if (_t_step == 0)
      return 1;
    return _old_val + 1;
  }

  else if (_test_type == "use_older_value")
  {
    if (_t_step == 0)
      return 1;
    return _old_val + _older_val;
  }

  else if (_test_type == "report_old")
    return _report_old;

  else if (_test_type == "custom_execute_on")
    return ++_execute_count;
  // This should not be attainable
  else
  {
    mooseError("Invalid test type.");
    return 0.0;
  }
}
