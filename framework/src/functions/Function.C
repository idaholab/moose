#include "Function.h"

template<>
InputParameters validParams<Function>()
{
  InputParameters params = validParams<MooseObject>();
  return params;
}

Function::Function(std::string name, MooseSystem & moose_system, InputParameters parameters) :
  MooseObject(name, moose_system, parameters)
{
}

Function::~Function()
{
}
