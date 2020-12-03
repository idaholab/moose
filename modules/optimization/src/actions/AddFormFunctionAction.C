#include "AddFormFunctionAction.h"

#include "Factory.h"
#include "FEProblemBase.h"
#include "FormFunction.h"

registerMooseAction("isopodApp", AddFormFunctionAction, "add_reporter");

InputParameters
AddFormFunctionAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Adds FormFunction objects for optimization routines.");
  return params;
}

AddFormFunctionAction::AddFormFunctionAction(InputParameters params) : MooseObjectAction(params) {}

void
AddFormFunctionAction::act()
{
  _problem->addReporter(_type, _name, _moose_object_pars);
}
