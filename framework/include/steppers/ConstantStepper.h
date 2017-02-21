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

#ifndef CONSTANTSTEPPER_H
#define CONSTANTSTEPPER_H

#include "Stepper.h"

class ConstantStepper;

template<>
InputParameters validParams<ConstantStepper>();

class ConstantStepper : public Stepper
{
public:
  ConstantStepper(const InputParameters & parameters);

  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

  virtual Real computeFailedDT() override;

protected:
  const Real & _input_dt;
};

#endif /* CONSTANTSTEPPER_H */
