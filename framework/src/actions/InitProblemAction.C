#include "InitProblemAction.h"

template<>
InputParameters validParams<InitProblemAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


InitProblemAction::InitProblemAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing InitProblemAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
InitProblemAction::act()
{
  std::cerr << "Acting on InitProblemAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
