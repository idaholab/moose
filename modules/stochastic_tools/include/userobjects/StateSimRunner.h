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

#ifndef STATESIMRUNNER_H
#define STATESIMRUNNER_H

#include <string>
#include "GeneralUserObject.h"
#include "StateProcessor.h"

class StateSimRunner;

template<>
InputParameters validParams<StateSimRunner>();

/**
 *
 */
class StateSimRunner : public GeneralUserObject
{
public:
  StateSimRunner(const InputParameters & parameters);
  virtual ~StateSimRunner();

  virtual void initialize();
  virtual void execute();
  virtual void finalize();

  Real getValue() const;
  //Real getValue() const { return 0; }

protected:
  string _model_path;
  StateProcessor _state_sim;
  int _next_state_time;
  bool ran_state_sim;
};

#endif /* STATESIMRUNNER_H */
