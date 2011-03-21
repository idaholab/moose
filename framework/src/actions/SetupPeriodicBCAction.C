#include "SetupPeriodicBCAction.h"

template<>
InputParameters validParams<SetupPeriodicBCAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


SetupPeriodicBCAction::SetupPeriodicBCAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing SetupPeriodicBCAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
SetupPeriodicBCAction::act()
{
  std::cerr << "Acting on SetupPeriodicBCAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
