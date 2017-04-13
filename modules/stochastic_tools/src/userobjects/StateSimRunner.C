/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimRunner.h"
#include <string>

template <>
InputParameters
validParams<StateSimRunner>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredParam<std::string>("model_path", "the model location for the state simulation");
  params.addParam<unsigned int>("seed", 0, "The start seed used for sampling");
  return params;
}

StateSimRunner::StateSimRunner(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _model_path("model_path"),
    _state_sim(100, parameters.get<unsigned int>("seed")), // todo max time step
    _next_state_time(-1),
    _ran_state_sim(false)
{
  //_state_sim.setMaxTime(_dt_max); todo
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
  if ((_next_state_time > 0) && (_t_step >= (int)_next_state_time))
  {
    _next_state_time = _state_sim.process(_t_step);
    _ran_state_sim = true;
  }
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
