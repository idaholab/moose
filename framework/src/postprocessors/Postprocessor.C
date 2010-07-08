#include "Postprocessor.h"

template<>
InputParameters validParams<Postprocessor>()
{
  InputParameters params = validParams<MooseObject>();
  return params;
}


Postprocessor::Postprocessor(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :MooseObject(name, moose_system, parameters)
{}


