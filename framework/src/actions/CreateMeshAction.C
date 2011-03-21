#include "CreateMeshAction.h"

template<>
InputParameters validParams<CreateMeshAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


CreateMeshAction::CreateMeshAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing CreateMeshAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
CreateMeshAction::act()
{
  std::cerr << "Acting on CreateMeshAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
