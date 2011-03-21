#include "AddBCAction.h"

template<>
InputParameters validParams<AddBCAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


AddBCAction::AddBCAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing AddBCAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
AddBCAction::act()
{
  std::cerr << "Acting on SetupMeshAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
