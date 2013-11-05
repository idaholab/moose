#include "SetupPredictorAction.h"
#include "Transient.h"
#include "Predictor.h"
#include "Factory.h"

template<>
InputParameters validParams<SetupPredictorAction>()
{
  InputParameters params = validParams<MooseObjectAction>();

  return params;
}

SetupPredictorAction::SetupPredictorAction(const std::string & name, InputParameters parameters) :
    MooseObjectAction(name, parameters)
{
}

SetupPredictorAction::~SetupPredictorAction()
{
}

void
SetupPredictorAction::act()
{
  if (_problem->isTransient())
  {
    Transient * transient = dynamic_cast<Transient *>(_executioner);
    if (transient == NULL)
      mooseError("You can setup time stepper only with executioners of transient type.");

    _moose_object_pars.set<FEProblem *>("_fe_problem") = _problem;
    _moose_object_pars.set<Transient *>("_executioner") = transient;
    Predictor * predictor = static_cast<Predictor *>(_factory.create(_type, "Predictor", _moose_object_pars));
    _problem->getNonlinearSystem().setPredictor(predictor);
  }
}
