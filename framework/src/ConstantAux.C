#include "ConstantAux.h"

template<>
Parameters valid_params<ConstantAux>()
{
  Parameters params;
  params.set<Real>("value")=0.0;
  return params;
}

ConstantAux::ConstantAux(std::string name,
                         Parameters parameters,
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
