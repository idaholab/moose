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
