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
  params.addParam<MooseEnum>("test_type", system_enum, "The value for testing (SYNCTIMES). Default == SYNCTIMES");
  return params;
}

StateSimTester::StateSimTester(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _state_sim_runner_ptr(getUserObject<StateSimRunner>("state_sim_runner")),
    _test_val_enum(parameters.get<MooseEnum>("test_type").getEnum<SystemEnum>())
{
  //_state_sim_runner_ptr = &&getUserObject<StateSimRunner>("state_sim_runner"),
}

Real
StateSimTester::getValue()
{
  switch (_test_val_enum)
  {
    case SYNCTIMES:
      return _state_sim_runner_ptr.getValue(); //todo
    default:
      return -1;
  }
}
