#include "BodyForce.h"

template<>
InputParameters validParams<BodyForce>()
{
  InputParameters params;
  params.set<Real>("value")=0.0;
  return params;
}

BodyForce::BodyForce(std::string name,
            InputParameters parameters,
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

