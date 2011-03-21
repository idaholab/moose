#include "MooseObjectAction.h"
#include "Factory.h"

template<>
InputParameters validParams<MooseObjectAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::string>("type", "A string representing the Moose Object that will be built by this Action");
  return params;
}

MooseObjectAction::MooseObjectAction(const std::string & name, InputParameters params) :
    Action(name, params),
    _type(getParam<std::string>("type")),
    _moose_object_pars(Factory::instance()->getValidParams(_type))  
{
}
