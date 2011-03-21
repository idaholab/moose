#include "CreateExecutionerAction.h"

template<>
InputParameters validParams<CreateExecutionerAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


CreateExecutionerAction::CreateExecutionerAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing CreateExecutionerAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
CreateExecutionerAction::act()
{
  std::cerr << "Acting on CreateExecutionerAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";
}
