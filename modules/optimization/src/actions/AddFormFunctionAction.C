#include "AddFormFunctionAction.h"

#include "Factory.h"
#include "FEProblemBase.h"
#include "FormFunction.h"

registerMooseAction("isopodApp", AddFormFunctionAction, "add_form_function");

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
  std::shared_ptr<FormFunction> ff =
      _factory.create<FormFunction>(_type, _name, _moose_object_pars);
  _problem->theWarehouse().add(ff);
}
