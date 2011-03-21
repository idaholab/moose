#include "AddICAction.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddICAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}


AddICAction::AddICAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
   std::cerr << "Constructing AddICAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
AddICAction::act()
{
  std::cerr << "Acting on AddICAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";

  std::vector<std::string> elements;
  Parser::tokenize(_name, elements);

  // The variable name will be the second to last element in the path name
  std::string & parent = elements[elements.size()-2];
  std::cout << "Parent: " << parent << "\n";
  _parser_handle._problem->addInitialCondition(_type, getShortName(), _moose_object_pars, parent);
}
