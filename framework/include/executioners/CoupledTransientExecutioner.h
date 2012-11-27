#ifndef COUPLEDTRANSIENTEXECUTIONER_H
#define COUPLEDTRANSIENTEXECUTIONER_H

#include "CoupledExecutioner.h"

class CoupledTransientExecutioner;

template<>
InputParameters validParams<CoupledTransientExecutioner>();

/**
 * This executioner first solves a Steady problem, then transfers the solution into the other problem and then runs the second one
 */
class CoupledTransientExecutioner : public CoupledExecutioner
{
public:
  CoupledTransientExecutioner(const std::string & name, InputParameters parameters);
  virtual ~CoupledTransientExecutioner();

  virtual void execute();

protected:
  /// Current time
  Real _time;
  /// Current delta t... or timestep size.
  Real _dt;

  unsigned int _t_step;
  unsigned int _n_steps;
};

#endif /* COUPLEDTRANSIENTEXECUTIONER_H */
