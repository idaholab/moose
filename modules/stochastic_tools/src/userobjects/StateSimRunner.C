/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimRunner.h"
#include "StateSimBase.h"
#include "StateExternalCouplingUO.h"
#include <string>

template <>
InputParameters
validParams<StateSimRunner>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredParam<std::string>("model_path", "the model location for the state simulation");
  params.addParam<unsigned int>("seed", 0, "The start seed used for sampling");
  params.addParam<UserObjectName>("ext_coupling_UO", "The StateExternalCoiuplingOP, vector postprocessor, for coupling events external to the state simulation");

  return params;
}

StateSimRunner::StateSimRunner(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _model_path(parameters.get<std::string>("model_path")),
    _state_sim_model(/*SateSimBase::DAY_TIME*/ StateSimBase::_DAY_TIME, getUserObject<StateExternalCouplingUO>("ext_coupling_UO")), //todo max time step
    _state_sim(parameters.get<unsigned int>("seed"), _state_sim_model),
    _next_state_time(-1),
    _ran_state_sim(false)
{
  _next_state_time = _state_sim.nextTime();
}

void
StateSimRunner::initialize()
{
}

void
StateSimRunner::execute()
{
  _ran_state_sim = false;
  TimespanH curTime = _fe_problem.time();                                               //if time is measured in hours;
  while ((_next_state_time > StateSimBase::ZERO_TIME) && (curTime >= _next_state_time)) //while loop incase more than one time event occurs in a time step
  {
    _next_state_time = _state_sim.processNext(curTime);
    _ran_state_sim = true;
  }

  if (_state_sim.hadCondEv())
    _ran_state_sim = true;
}

Real
StateSimRunner::getValue() const
{
  return _ran_state_sim;
}

void
StateSimRunner::finalize()
{
}
