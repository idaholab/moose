#include "CoupledAux.h"

template<>
Parameters valid_params<CoupledAux>()
{
  Parameters params;
  params.set<Real>("value")=0.0;
  return params;
}

CoupledAux::CoupledAux(std::string name,
                         Parameters parameters,
                         std::string var_name,
                         std::vector<std::string> coupled_to,
                         std::vector<std::string> coupled_as)
  :AuxKernel(name, parameters, var_name, coupled_to, coupled_as),
   _coupled(coupled("coupled")),
   _coupled_val(coupledValAux("coupled")),
   _value(_parameters.get<Real>("value"))
{}

Real
CoupledAux::computeValue()
{
  return _coupled_val+_value;
}
