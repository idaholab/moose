//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SetupPredictorAction.h"
#include "Transient.h"
#include "Predictor.h"
#include "Factory.h"
#include "NonlinearSystemBase.h"

template <>
InputParameters
validParams<SetupPredictorAction>()
{
  InputParameters params = validParams<MooseObjectAction>();

  return params;
}

SetupPredictorAction::SetupPredictorAction(InputParameters parameters)
  : MooseObjectAction(parameters)
{
}

void
SetupPredictorAction::act()
{
  if (_problem->isTransient())
  {
    Transient * transient = dynamic_cast<Transient *>(_executioner.get());
    if (transient == NULL)
      mooseError("You can setup time stepper only with executioners of transient type.");

    _moose_object_pars.set<FEProblemBase *>("_fe_problem_base") = _problem.get();
    _moose_object_pars.set<Transient *>("_executioner") = transient;
    std::shared_ptr<Predictor> predictor =
        _factory.create<Predictor>(_type, "Predictor", _moose_object_pars);
    _problem->getNonlinearSystemBase().setPredictor(predictor);
  }
}
