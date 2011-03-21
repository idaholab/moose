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
}

void
EmptyAction::act()
{
}
