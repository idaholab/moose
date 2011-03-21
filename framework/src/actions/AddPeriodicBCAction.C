#include "AddPeriodicBCAction.h"

template<>
InputParameters validParams<AddPeriodicBCAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


AddPeriodicBCAction::AddPeriodicBCAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing AddPeriodicBCAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
AddPeriodicBCAction::act()
{
  std::cerr << "Acting on AddPeriodicBCAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
