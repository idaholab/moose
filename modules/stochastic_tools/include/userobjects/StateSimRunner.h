//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
