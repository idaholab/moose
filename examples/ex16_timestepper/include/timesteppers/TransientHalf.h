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

#ifndef TRANSIENTHALF_H
#define TRANSIENTHALF_H

#include "TimeStepper.h"

// Forward Declarations
class TransientHalf;

template<>
InputParameters validParams<TransientHalf>();

/**
 * This class cuts the timestep in half at every iteration
 * until it reaches a user-specified minimum value.
 */
class TransientHalf : public TimeStepper
{
public:

  TransientHalf(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeInitialDT();

  virtual Real computeDT();

private:
  Real _ratio;
  Real _min_dt;
};

#endif //TRANSIENTHALF_H
