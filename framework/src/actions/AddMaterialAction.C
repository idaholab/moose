#include "AddMaterialAction.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddMaterialAction>()
{
  return validParams<MooseObjectAction>();
}

AddMaterialAction::AddMaterialAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddMaterialAction::act()
{
  _parser_handle._problem->addMaterial(_type, getShortName(), _moose_object_pars);  
}
