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

#include "SetupTimeStepperAction.h"
#include "Transient.h"
#include "Factory.h"
#include "TimeStepper.h"

template <>
InputParameters
validParams<SetupTimeStepperAction>()
{
  InputParameters params = validParams<MooseObjectAction>();

  return params;
}

SetupTimeStepperAction::SetupTimeStepperAction(InputParameters parameters)
  : MooseObjectAction(parameters)
{
}

void
SetupTimeStepperAction::act()
{
  if (_problem->isTransient())
  {
    Transient * transient = dynamic_cast<Transient *>(_executioner.get());
    if (transient == NULL)
      mooseError("You can setup time stepper only with executioners of transient type.");

    _moose_object_pars.set<FEProblemBase *>("_fe_problem_base") = _problem.get();
    _moose_object_pars.set<Transient *>("_executioner") = transient;
    std::shared_ptr<TimeStepper> ts =
        _factory.create<TimeStepper>(_type, "TimeStepper", _moose_object_pars);
    transient->setTimeStepper(ts);
  }
}
