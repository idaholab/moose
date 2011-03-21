#include "AddICAction.h"

template<>
InputParameters validParams<AddICAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


AddICAction::AddICAction(const std::string & name, InputParameters params) :
    Action(name, params)
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
}
