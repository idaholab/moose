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

#ifndef LIMITSTEPPER_H
#define LIMITSTEPPER_H

#include "Stepper.h"

class LimitStepper;

template<>
InputParameters validParams<LimitStepper>();

/**
 * Forces dt to be between min and max
 */
class LimitStepper : public Stepper
{
public:
  LimitStepper(const InputParameters & parameters);

  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

  virtual Real computeFailedDT() override;

protected:
  const Real & _incoming_stepper_dt;

  const Real & _min;
  const Real & _max;
};

#endif /* LIMITSTEPPER_H */
