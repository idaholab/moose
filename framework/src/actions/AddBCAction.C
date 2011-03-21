#include "AddBCAction.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddBCAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.addRequiredParam<std::string>("variable", "The BC Name used in your model");
  params.addRequiredParam<std::vector<int> >("boundary", "The boundary number from your input mesh which corresponds to this boundary");
  return params;
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
