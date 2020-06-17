//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestControl.h"
#include "FEProblem.h"
#include "InputParameterWarehouse.h"

registerMooseObject("MooseTestApp", TestControl);

InputParameters
TestControl::validParams()
{
  InputParameters params = Control::validParams();

  MooseEnum test_type("real variable point tid_warehouse_error disable_executioner connection "
                      "alias mult execflag_error");
  params.addRequiredParam<MooseEnum>(
      "test_type", test_type, "Indicates the type of test to perform");
  params.addParam<std::string>(
      "parameter",
      "The input parameter(s) to control. Specify a single parameter name and all "
      "parameters in all objects matching the name will be updated");

  return params;
}

TestControl::TestControl(const InputParameters & parameters)
  : Control(parameters), _test_type(getParam<MooseEnum>("test_type")), _alias("this/is/alias")
{
  if (_test_type == "real")
    getControllableValue<Real>("parameter");

  else if (_test_type == "variable")
    getControllableValue<NonlinearVariableName>("parameter");

  else if (_test_type == "tid_warehouse_error")
    _fe_problem.getControlWarehouse().initialSetup(12345);

  else if (_test_type == "disable_executioner")
    getControllableValue<bool>("parameter");

  else if (_test_type == "connection")
  {
    MooseObjectParameterName primary(MooseObjectName("Kernels", "diff"), "coef");
    MooseObjectParameterName secondary(MooseObjectName("BCs", "left"), "value");
    _app.getInputParameterWarehouse().addControllableParameterConnection(primary, secondary);
  }

  else if (_test_type == "alias")
  {
    MooseObjectParameterName secondary(MooseObjectName("BCs", "left"), "value");
    _app.getInputParameterWarehouse().addControllableParameterAlias(_alias, secondary);
  }

  else if (_test_type == "mult")
    getControllableValue<Real>("parameter");

  else if (_test_type != "point" && _test_type != "execflag_error")
    mooseError("Unknown test type.");
}

void
TestControl::execute()
{
  if (_test_type == "point")
    setControllableValue<Point>("parameter", Point(0.25, 0.25));

  else if (_test_type == "connection")
    setControllableValue<Real>("parameter", 0.2);

  else if (_test_type == "alias")
    setControllableValueByName<Real>(_alias, 0.42);

  else if (_test_type == "mult")
  {
    const Real & val = getControllableValue<Real>("parameter");
    setControllableValue<Real>("parameter", val * 3);
  }

  else if (_test_type == "execflag_error")
    setControllableValueByName<std::vector<std::string>>(
        "MultiApps", "sub", "cli_args", {"Mesh/nx=2"});
}
