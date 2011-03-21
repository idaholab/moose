#include "SetupOutputAction.h"

template<>
InputParameters validParams<SetupOutputAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


SetupOutputAction::SetupOutputAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing SetupOutputAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
SetupOutputAction::act()
{
  std::cerr << "Acting on SetupOutputAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
