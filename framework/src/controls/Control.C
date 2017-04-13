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
  params.registerBase("Control");

  params.set<MultiMooseEnum>("execute_on") = Control::getExecuteOptions();

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
  return MultiMooseEnum("none=0x00 initial=0x01 linear=0x02 nonlinear=0x04 timestep_end=0x08 "
                        "timestep_begin=0x10 custom=0x100 subdomain=0x200",
                        "initial timestep_end");
}
