//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BinaryTestExecutor.h"

registerMooseObject("MooseApp", BinaryTestExecutor);

InputParameters
BinaryTestExecutor::validParams()
{
  InputParameters params = Executor::validParams();
  params.addParam<ExecutorName>("inner1", "inner executor name to run");
  params.addParam<ExecutorName>("inner2", "another inner executor name to run");
  params.addParam<bool>(
      "fail_early", false, "true to cause executor to fail execution before inner executors run");
  params.addParam<bool>(
      "fail_late", false, "true to cause executor to fail execution after inner executors run");
  params.addParam<bool>("return_early_inner_fail",
                        false,
                        "true to cause executor to return early after inner executor failures");
  return params;
}

BinaryTestExecutor::BinaryTestExecutor(const InputParameters & parameters)
  : Executor(parameters), _inner1(getExecutor("inner1")), _inner2(getExecutor("inner2"))
{
}

Executor::Result
BinaryTestExecutor::run()
{
  Result & result = newResult();
  _console << "BinaryTestExecutor " << name() << " BEGIN\n" << std::flush;

  if (getParam<bool>("fail_early"))
  {
    result.fail("manual early fail");
    _console << "BinaryTestExecutor " << name() << " END EARLY\n" << std::flush;
    return result;
  }

  if (isParamValid("inner1"))
  {
    bool converged = result.record("inner1", _inner1.exec());
    if (getParam<bool>("return_early_inner_fail") && !converged)
    {
      _console << "BinaryTestExecutor " << name() << " END EARLY\n" << std::flush;
      return result;
    }
  }

  if (isParamValid("inner2"))
    result.record("inner2", _inner2.exec());

  if (getParam<bool>("fail_late"))
    result.fail("manual late fail");

  _console << "BinaryTestExecutor " << name() << " END\n" << std::flush;
  return result;
}
