#include "CoupledAux.h"

template<>
InputParameters validParams<CoupledAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.set<Real>("value")=0.0;
  params.set<std::string>("operator")='+';
  return params;
}

CoupledAux::CoupledAux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :AuxKernel(name, moose_system, parameters),
   _coupled(coupled("coupled")),
   _coupled_val(coupledValAux("coupled")),
   _value(_parameters.get<Real>("value")),
   _operator(_parameters.get<std::string>("operator"))
{}


Real
CoupledAux::computeValue()
{
  if (_operator == "+")
    return _coupled_val+_value;
  else if (_operator == "-")
    return _coupled_val-_value;
  else if (_operator == "*")
    return _coupled_val*_value;
  else if (_operator == "/")
    return _coupled_val/_value;
  else
    mooseError("Unknown operator");
}
