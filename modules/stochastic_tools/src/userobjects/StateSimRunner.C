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

#include "StateSimRunner.h"
#include <string>

template <>
InputParameters
validParams<StateSimRunner>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredParam<std::string>("model_path", "the model location for the state simulation");
  return params;
}

StateSimRunner::StateSimRunner(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _model_path("model_path"),
    _state_sim(100), //todo max time step
    _next_state_time(-1)
{
  //_state_sim.setMaxTime(_dt_max); todo
  _next_state_time = _state_sim.nextTime();
}

StateSimRunner::~StateSimRunner()
{
}

void
StateSimRunner::initialize()
{
  //
}

void
StateSimRunner::execute()
{
  ran_state_sim = false;
  if ((_next_state_time > 0) && (_t_step >= _next_state_time))
  {
    _next_state_time = _state_sim.process(_t_step);
    ran_state_sim = true;
  }
}

Real
StateSimRunner::getValue() const
{
  return ran_state_sim;
}

void
StateSimRunner::finalize()
{
  //
}
