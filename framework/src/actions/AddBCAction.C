#include "AddBCAction.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddBCAction>()
{
  return validParams<MooseObjectAction>();
}

AddBCAction::AddBCAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddBCAction::act()
{
  if (Parser::pathContains(_name, "BCs"))
    _parser_handle._problem->addBoundaryCondition(_type, getShortName(), _moose_object_pars);
  else
    _parser_handle._problem->addAuxBoundaryCondition(_type, getShortName(), _moose_object_pars);  
}
