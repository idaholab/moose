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
   std::cerr << "Constructing AddBCAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
AddBCAction::act()
{
  std::cerr << "Acting on AddBCAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";

  if (Parser::pathContains(_name, "BCs"))
    _parser_handle._problem->addBoundaryCondition(_type, getShortName(), _moose_object_pars);
  else
    _parser_handle._problem->addAuxBoundaryCondition(_type, getShortName(), _moose_object_pars);  
}
