/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMRUNNER_H
#define STATESIMRUNNER_H

#include "GeneralUserObject.h"
#include "StateProcessor.h"
#include <string>

/**
 Object to couple the Moose time steps with the State Sim Discrete Events.
 */
class StateSimRunner;

template<>
InputParameters validParams<StateSimRunner>();

/**
 Object to start a State Simulation and manage the moose timesteping with state event times.
 */
class StateSimRunner : public GeneralUserObject
{
public:
  /**
   * Main constructor to run a StateSim model.
   * @param parameters - user object parameters contains model_path, external_coupling_uo and seed
   */
  StateSimRunner(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize()override;

  Real getValue() const;

protected:
  std::string _model_path;
  StateSimModel _state_sim_model;
  StateProcessor _state_sim;
  TimespanH _next_state_time;
  bool _ran_state_sim;
};

#endif /* STATESIMRUNNER_H */
