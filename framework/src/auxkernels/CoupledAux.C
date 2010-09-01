#include "CoupledAux.h"

template<>
InputParameters validParams<CoupledAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredCoupledVar("coupled", "Coupled Value for Calculation");
  
  params.set<Real>("value")=0.0;
  params.set<std::string>("operator")='+';
  return params;
}

CoupledAux::CoupledAux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :AuxKernel(name, moose_system, parameters),
   _value(getParam<Real>("value")),
   _operator(getParam<std::string>("operator")),
   _coupled(coupled("coupled")),
   _coupled_val(coupledValue("coupled"))
{}


Real
CoupledAux::computeValue()
{
  if (_operator == "+")
    return _coupled_val[_qp]+_value;
  else if (_operator == "-")
    return _coupled_val[_qp]-_value;
  else if (_operator == "*")
    return _coupled_val[_qp]*_value;
  else if (_operator == "/")
    return _coupled_val[_qp]/_value;
  else
    mooseError("Unknown operator");
}
