#include "AddAuxVariableAction.h"

template<>
InputParameters validParams<AddAuxVariableAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


AddAuxVariableAction::AddAuxVariableAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing AddAuxVariableAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
AddAuxVariableAction::act()
{
  std::cerr << "Acting on AddAuxVariableAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
