#include "SetupMeshAction.h"

template<>
InputParameters validParams<SetupMeshAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


SetupMeshAction::SetupMeshAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing SetupMeshAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
SetupMeshAction::act()
{
  std::cerr << "Acting on SetupMeshAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
