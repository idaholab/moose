#include "CopyNodalVariablesAction.h"

template<>
InputParameters validParams<CopyNodalVariablesAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


CopyNodalVariablesAction::CopyNodalVariablesAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing CopyNodalVariablesAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
CopyNodalVariablesAction::act()
{
  std::cerr << "Acting on CopyNodalVariablesAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
