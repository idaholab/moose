#include "AddAuxBCAction.h"

template<>
InputParameters validParams<AddAuxBCAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


AddAuxBCAction::AddAuxBCAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing AddAuxBCAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
AddAuxBCAction::act()
{
  std::cerr << "Acting on AddAuxBCAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
