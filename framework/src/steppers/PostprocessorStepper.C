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

#include "PostprocessorStepper.h"
#include "FEProblem.h"
#include "Transient.h"
#include "MooseApp.h"

template<>
InputParameters validParams<PostprocessorStepper>()
{
  InputParameters params = validParams<Stepper>();

  params.addParam<StepperName>("incoming_stepper", "OPTIONAL: If provided this Stepper will compute the minimum of the incoming_stepper and the Postprocessor value.");

  params.addRequiredParam<PostprocessorName>("postprocessor", "The Postprocessor to use as dt");

  return params;
}

PostprocessorStepper::PostprocessorStepper(const InputParameters & parameters) :
    Stepper(parameters),

    // In the case where "incoming_stepper" is not defined this will automatically get a max() value
    _incoming_stepper_dt(getStepperDT("incoming_stepper")),
    _pp_value(getPostprocessorValue("postprocessor"))
{
}

Real
PostprocessorStepper::computeInitialDT()
{
  return computeDT();
}

Real
PostprocessorStepper::computeDT()
{
  return std::min(_incoming_stepper_dt, _pp_value);
}

Real
PostprocessorStepper::computeFailedDT()
{
  // If we're currently filtering then just pass it through
  if ( _pars.isParamSetByUser("incoming_stepper") )
    return _incoming_stepper_dt;
  else // We are a root - so try to do something
    return 0.5 * _dt[0];
}

PostprocessorStepper::~PostprocessorStepper()
{
}
