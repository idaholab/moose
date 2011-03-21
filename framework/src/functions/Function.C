#include "Function.h"

template<>
InputParameters validParams<Function>()
{
  InputParameters params = validParams<MooseObject>();

  params.addPrivateParam<std::string>("built_by_action", "add_function");
  return params;
}

Function::Function(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters)
{
}

Function::~Function()
{
}

RealGradient 
Function::gradient(Real /*t*/, const Point & /*p*/)
{
  return RealGradient(0, 0, 0);
}
