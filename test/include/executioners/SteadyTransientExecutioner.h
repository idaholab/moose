#ifndef STEADYTRANSIENTEXECUTIONER_H
#define STEADYTRANSIENTEXECUTIONER_H

#include "CoupledExecutioner.h"

class SteadyTransientExecutioner;

template<>
InputParameters validParams<SteadyTransientExecutioner>();

/**
 * This executioner first solves a Steady problem, then transfers the solution into the other problem and then runs the second one
 */
class SteadyTransientExecutioner : public CoupledExecutioner
{
public:
  SteadyTransientExecutioner(const std::string & name, InputParameters parameters);
  virtual ~SteadyTransientExecutioner();

  virtual void execute();

protected:

};

#endif /* STEADYTRANSIENTEXECUTIONER_H */
