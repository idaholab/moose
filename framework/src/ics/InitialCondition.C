#include "InitialCondition.h"

template<>
InputParameters validParams<InitialCondition>()
{
  InputParameters params;
  params.addParam<std::string>("var_name", "The variable this InitialCondtion is supposed to provide values for.");
  return params;
}

InitialCondition::InitialCondition(std::string name, MooseSystem & moose_system, InputParameters parameters) :
  MooseObject(name, moose_system, parameters),
  _var_name(parameters.get<std::string>("var_name"))
{
}

InitialCondition::~InitialCondition()
{
}
