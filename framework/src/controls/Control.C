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
