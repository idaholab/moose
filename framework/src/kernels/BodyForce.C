#include "BodyForce.h"
 
template<>
Parameters valid_params<BodyForce>()
{
  Parameters params;
  params.set<Real>("value")=0.0;
  return params;
}

BodyForce::BodyForce(std::string name,
            Parameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to,
            std::vector<std::string> coupled_as)
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as),
    _value(_parameters.get<Real>("value"))
  {}

Real
BodyForce::computeQpResidual()
  {
    return _phi[_i][_qp]*-_value;
  }

