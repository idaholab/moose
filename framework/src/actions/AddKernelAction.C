#include "AddKernelAction.h"

template<>
InputParameters validParams<AddKernelAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


AddKernelAction::AddKernelAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing AddKernelAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
AddKernelAction::act()
{
  std::cerr << "Acting on AddKernelAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
