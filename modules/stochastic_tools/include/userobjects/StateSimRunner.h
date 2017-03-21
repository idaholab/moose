/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMRUNNER_H
#define STATESIMRUNNER_H

#include <string>
#include "GeneralUserObject.h"
#include "StateProcessor.h"

class StateSimRunner;

template <>
InputParameters validParams<StateSimRunner>();

/**
 Object to start a State Simulation and manage the moose timesteping with state event times.
 */
class StateSimRunner : public GeneralUserObject
{
public:
  StateSimRunner(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  Real getValue() const;
  // Real getValue() const { return 0; }

protected:
  std::string _model_path;
  StateProcessor _state_sim;
  unsigned int _next_state_time;
  bool _ran_state_sim;
};

#endif /* STATESIMRUNNER_H */
