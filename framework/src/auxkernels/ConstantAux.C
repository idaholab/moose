#include "ConstantAux.h"

template<>
InputParameters valid_params<ConstantAux>()
{
  InputParameters params;
  params.set<Real>("value")=0.0;
  return params;
}

ConstantAux::ConstantAux(std::string name,
                         InputParameters parameters,
                         std::string var_name,
                         std::vector<std::string> coupled_to,
                         std::vector<std::string> coupled_as)
  :AuxKernel(name, parameters, var_name, coupled_to, coupled_as),
   _value(_parameters.get<Real>("value"))
{}


Real
ConstantAux::computeValue()
{
  return _value;
}
