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
  MooseUtils::setExecuteOnFlags(params, {EXEC_INITIAL, EXEC_TIMESTEP_END});
  params.registerBase("Control");
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
    _fe_problem(*parameters.get<FEProblemBase *>("_fe_problem_base")),
    _input_parameter_warehouse(_app.getInputParameterWarehouse())
{
}

MultiMooseEnum
Control::getExecuteOptions()
{
  ::mooseDeprecated("The 'getExecuteOptions' was replaced by MooseUtils::createExecuteOnEnum "
                    "because MOOSE was updated to use a MultiMooseEnum for the execute flags and "
                    "the new function provides additional arguments for modification of the enum.");
  return MooseUtils::createExecuteOnEnum({EXEC_INITIAL, EXEC_TIMESTEP_END});
}
