#include "Function.h"

template<>
InputParameters validParams<Function>()
{
  InputParameters params = validParams<Object>();
  return params;
}

Function::Function(const std::string & name, InputParameters parameters) :
    Object(name, parameters)
{
}

Function::~Function()
{
}

RealGradient 
Function::gradient(Real /*t*/, Real /*x*/, Real /*y*/, Real /*z*/)
{
  return RealGradient(0, 0, 0);
}
