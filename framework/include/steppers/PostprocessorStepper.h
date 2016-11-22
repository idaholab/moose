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

#ifndef POSTPROCESSORSTEPPER_H
#define POSTPROCESSORSTEPPER_H

#include "Stepper.h"
#include "LinearInterpolation.h"

class PostprocessorStepper;

template<>
InputParameters validParams<PostprocessorStepper>();

/**
 * Choose dt based on a list of dt's and times
 */
class PostprocessorStepper : public Stepper
{
public:
  PostprocessorStepper(const InputParameters & parameters);
  virtual ~PostprocessorStepper();

  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

  virtual Real computeFailedDT() override;

protected:
  const Real & _incoming_stepper_dt;

  const PostprocessorValue & _pp_value;
};

#endif /* POSTPROCESSORSTEPPER_H */
