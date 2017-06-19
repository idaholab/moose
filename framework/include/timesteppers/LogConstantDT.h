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

#ifndef LOGCONSTANTDT_H
#define LOGCONSTANTDT_H

#include "TimeStepper.h"

class LogConstantDT;

template <>
InputParameters validParams<LogConstantDT>();

/** Simple time-stepper which imposes a time step constant in the logarithmic
 * space */
class LogConstantDT : public TimeStepper
{
public:
  LogConstantDT(const InputParameters & parameters);

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

private:
  /// distance between two time steps in the logarithmic space
  const Real _log_dt;

  /// first time step (in absolute time)
  const Real _first_dt;

  const Real _dt_factor;

  const Real _growth_factor;
};

#endif /* LOGCONSTANTDT_H */
