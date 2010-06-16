#include "ExampleAux.h"

template<>
InputParameters validParams<ExampleAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.set<Real>("value")=0.0;
  return params;
}

ExampleAux::ExampleAux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :AuxKernel(name, moose_system, parameters),
   _coupled_val(coupledValAux("coupled")),
   _value(_parameters.get<Real>("value"))
{}


Real
ExampleAux::computeValue()
{
  return _coupled_val + _value;
}
