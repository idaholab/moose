//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TRANSIENTHALF_H
#define TRANSIENTHALF_H

#include "TimeStepper.h"

// Forward Declarations
class TransientHalf;

template <>
InputParameters validParams<TransientHalf>();

/**
 * This class cuts the timestep in half at every iteration
 * until it reaches a user-specified minimum value.
 */
class TransientHalf : public TimeStepper
{
public:
  TransientHalf(const InputParameters & parameters);

protected:
  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

private:
  Real _ratio;
  Real _min_dt;
};

#endif // TRANSIENTHALF_H
