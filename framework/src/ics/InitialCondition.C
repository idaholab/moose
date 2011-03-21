#include "InitialCondition.h"

template<>
InputParameters validParams<InitialCondition>()
{
  InputParameters params;
  params.addParam<std::string>("var_name", "The variable this InitialCondtion is supposed to provide values for.");
  return params;
}

InitialCondition::InitialCondition(const std::string & name, InputParameters parameters) :
  Object(name, parameters),
  _var_name(getParam<std::string>("var_name"))
{
}

InitialCondition::~InitialCondition()
{
}

