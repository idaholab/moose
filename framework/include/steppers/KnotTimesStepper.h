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

#ifndef KNOTTIMESSTEPPER_H
#define KNOTTIMESSTEPPER_H

#include "Stepper.h"
#include "LinearInterpolation.h"

class KnotTimesStepper;

template<>
InputParameters validParams<KnotTimesStepper>();

/**
 * Choose dt based on a list of dt's and times
 */
class KnotTimesStepper : public Stepper
{
public:
  KnotTimesStepper(const InputParameters & parameters);

  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

  virtual Real computeFailedDT() override;

protected:
  const Real & _incoming_stepper_dt;

  const std::vector<Real> & _times;
  const std::vector<Real> & _dts;
};

#endif /* KNOTTIMESSTEPPER_H */
