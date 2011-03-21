#include "AddVariableAction.h"

template<>
InputParameters validParams<AddVariableAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


AddVariableAction::AddVariableAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing AddVariableAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
AddVariableAction::act()
{
  std::cerr << "Acting on AddVariableAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
