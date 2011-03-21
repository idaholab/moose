#include "AddMeshModifierAction.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddMeshModifierAction>()
{
  return validParams<MooseObjectAction>();
}

AddMeshModifierAction::AddMeshModifierAction(const std::string & name, InputParameters params) :
  MooseObjectAction(name, params)
{
}

void
AddMeshModifierAction::act() 
{
  _parser_handle._mesh->addMeshModifer(_type, getShortName(), _moose_object_pars);
}

