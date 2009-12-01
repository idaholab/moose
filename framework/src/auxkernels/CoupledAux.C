#include "CoupledAux.h"

template<>
InputParameters valid_params<CoupledAux>()
{
  InputParameters params;
  params.set<Real>("value")=0.0;
  params.set<std::string>("operator")='+';
  return params;
}

CoupledAux::CoupledAux(std::string name,
                         InputParameters parameters,
                         std::string var_name,
                         std::vector<std::string> coupled_to,
                         std::vector<std::string> coupled_as)
  :AuxKernel(name, parameters, var_name, coupled_to, coupled_as),
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
