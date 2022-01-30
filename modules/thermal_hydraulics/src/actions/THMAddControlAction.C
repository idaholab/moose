#include "THMAddControlAction.h"
#include "THMProblem.h"
#include "FEProblem.h"
#include "Factory.h"
#include "Control.h"
#include "THMControl.h"

registerMooseAction("ThermalHydraulicsApp", THMAddControlAction, "add_control");

InputParameters
THMAddControlAction::validParams()
{
  InputParameters params = AddControlAction::validParams();
  return params;
}

THMAddControlAction::THMAddControlAction(InputParameters parameters) : AddControlAction(parameters)
{
}

void
THMAddControlAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
  {
    _moose_object_pars.addPrivateParam<FEProblemBase *>("_fe_problem_base", _problem.get());
    _moose_object_pars.addPrivateParam<THMProblem *>("_thm_problem", thm_problem);
    std::shared_ptr<Control> control = _factory.create<Control>(_type, _name, _moose_object_pars);
    _problem->getControlWarehouse().addObject(control);
  }
  else
    AddControlAction::act();
}
