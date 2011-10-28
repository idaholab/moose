#include "AddComponentAction.h"
#include "Simulation.h"

template <>
InputParameters validParams<AddComponentAction>()
{
  InputParameters params = validParams<R7Action>();
  return params;
}

AddComponentAction::AddComponentAction(const std::string & name, InputParameters params) :
    R7Action(name, params)
{
}

void
AddComponentAction::act()
{
  std::cerr << "AddComponent: " << _type << " " << getShortName() << std::endl;
  _simulation.addComponent(_type, getShortName(), _r7_obj_pars);
}
