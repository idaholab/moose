#include "EmptyAction.h"

template<>
InputParameters validParams<EmptyAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


EmptyAction::EmptyAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing EmptyAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
EmptyAction::act()
{
  std::cerr << "Acting on EmptyAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
