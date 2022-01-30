#include "AddClosuresAction.h"
#include "THMProblem.h"
#include "ClosuresBase.h"

registerMooseAction("ThermalHydraulicsApp", AddClosuresAction, "THM:add_closures");

InputParameters
AddClosuresAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Adds a Closures object.");
  return params;
}

AddClosuresAction::AddClosuresAction(InputParameters params) : MooseObjectAction(params) {}

void
AddClosuresAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
  {
    _moose_object_pars.set<THMProblem *>("_thm_problem") = thm_problem;
    _moose_object_pars.set<Logger *>("_logger") = &(thm_problem->log());

    thm_problem->addClosures(_type, _name, _moose_object_pars);
  }
}
