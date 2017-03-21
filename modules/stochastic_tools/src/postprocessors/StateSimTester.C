/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimTester.h"
#include "StateSimTester.h"

#include "libmesh/system.h"
#include "StateSimRunner.h"

template <>
InputParameters
validParams<StateSimTester>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<UserObjectName>("state_sim_runner", "The StateSimRunner to test.");
  MooseEnum system_enum("SYNCTIMES", "SYNCTIMES");
  params.addParam<MooseEnum>(
      "test_type", system_enum, "The value for testing (SYNCTIMES). Default == SYNCTIMES");
  return params;
}

StateSimTester::StateSimTester(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _state_sim_runner_ptr(getUserObject<StateSimRunner>("state_sim_runner")),
    _test_val_enum(parameters.get<MooseEnum>("test_type").getEnum<SystemEnum>())
{
}

Real
StateSimTester::getValue()
{
  switch (_test_val_enum)
  {
    case SYNCTIMES:
      return _state_sim_runner_ptr.getValue();
    default:
      return -1;
  }
}
