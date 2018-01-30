//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

    _moose_object_pars.set<SubProblem *>("_subproblem") = _problem.get();
    _moose_object_pars.set<FEProblemBase *>("_fe_problem_base") = _problem.get();
    _moose_object_pars.set<Transient *>("_executioner") = transient;
    std::shared_ptr<TimeStepper> ts =
        _factory.create<TimeStepper>(_type, "TimeStepper", _moose_object_pars);
    transient->setTimeStepper(ts);
  }
}
