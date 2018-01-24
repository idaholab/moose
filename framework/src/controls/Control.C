//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Control.h"

template <>
InputParameters
validParams<Control>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<TransientInterface>();
  params += validParams<SetupInterface>();
  params += validParams<FunctionInterface>();

  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  params.registerBase("Control");

  params.addParam<std::vector<std::string>>(
      "depends_on",
      "The Controls that this control relies upon (i.e. must execute before this one)");

  return params;
}

Control::Control(const InputParameters & parameters)
  : MooseObject(parameters),
    TransientInterface(this),
    SetupInterface(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _depends_on(getParam<std::vector<std::string>>("depends_on")),
    _input_parameter_warehouse(_app.getInputParameterWarehouse())
{
}

MultiMooseEnum
Control::getExecuteOptions()
{
  ::mooseDeprecated("The 'getExecuteOptions' was replaced by the ExecFlagEnum class because MOOSE "
                    "was updated to use this for the execute flags and the new function provides "
                    "additional arguments for modification of the enum.");
  ExecFlagEnum execute_on = MooseUtils::getDefaultExecFlagEnum();
  execute_on = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  return execute_on;
}
