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

#ifndef FIXEDTIMESSTEPPER_H
#define FIXEDTIMESSTEPPER_H

#include "Stepper.h"

class FixedTimesStepper;

template<>
InputParameters validParams<FixedTimesStepper>();

/**
 * Forces the steps to hit the specified times exactly
 */
class FixedTimesStepper : public Stepper
{
public:
  FixedTimesStepper(const InputParameters & parameters);

  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

  virtual Real computeFailedDT() override;

protected:
  /// If not coupled then this will be max()
  const Real & _incoming_stepper_dt;

  const std::vector<Real> & _times;
};

#endif /* FIXEDTIMESSTEPPER_H */
