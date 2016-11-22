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

#ifndef INITIALSTEPSSTEPPER_H
#define INITIALSTEPSSTEPPER_H

#include "Stepper.h"

class InitialStepsStepper;

template<>
InputParameters validParams<InitialStepsStepper>();

/**
 * Cuts the incoming dt for the first N steps
 */
class InitialStepsStepper : public Stepper
{
public:
  InitialStepsStepper(const InputParameters & parameters);
  virtual ~InitialStepsStepper();

  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

  virtual Real computeFailedDT() override;

protected:
  const Real & _incoming_stepper_dt;

  const Real & _input_dt;

  const unsigned int & _n_steps;
};

#endif /* INITIALSTEPSSTEPPER_H */
