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

#include "Stepper.h"
#include "FEProblemBase.h"
#include "Transient.h"
#include "MooseApp.h"

template<>
InputParameters validParams<Stepper>()
{
  InputParameters params = validParams<MooseObject>();

  params.registerBase("Stepper");

  // Controls the name of the output from this stepper
  // If this is left blank then the name will default to the name of the object
  params.addPrivateParam<StepperName>("_output_name", "");

  return params;
}

Stepper::Stepper(const InputParameters & parameters) :
    MooseObject(parameters),
    Restartable(parameters, "Steppers"),
    StepperInterface(this),
    PostprocessorInterface(this),
    _fe_problem_base(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _executioner(*dynamic_cast<Transient*>(_app.getExecutioner())),
    _factory(_app.getFactory()),
    _output_name(getParam<StepperName>("_output_name") != "" ? getParam<StepperName>("_output_name") : name()),
    _stepper_info(_fe_problem_base.getStepperInfo()),
    _step_count(_stepper_info._step_count),
    _time(_stepper_info._time),
    _dt(_stepper_info._dt),
    _nonlin_iters(_stepper_info._nonlin_iters),
    _lin_iters(_stepper_info._lin_iters),
    _converged(_stepper_info._converged),
    _solve_time_secs(_stepper_info._solve_time_secs),
    _soln_nonlin(_stepper_info._soln_nonlin),
    _soln_aux(_stepper_info._soln_aux),
    _soln_predicted(_stepper_info._soln_predicted),
    _backup(_stepper_info._backup),
    _restore(_stepper_info._restore),
    _restore_time(_stepper_info._restore_time)
{
}

Stepper::~Stepper()
{
}

void
Stepper::setOutputName(const StepperName & output_name)
{
  _output_name = output_name;

  // Need to tell the Interface about this as well...
  setSuppliedItemName(output_name);
}
