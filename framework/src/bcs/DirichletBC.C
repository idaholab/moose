#include "DirichletBC.h"

template<>
Parameters valid_params<DirichletBC>()
{
  Parameters params;
  params.set<Real>("value")=0.0;
  return params;
}

DirichletBC::DirichletBC(std::string name, Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(name, parameters, var_name, false, boundary_id, coupled_to, coupled_as),
    _value(_parameters.get<Real>("value"))
  {}

Real
DirichletBC::computeQpResidual()
  {
    return _u_face[_qp]-_value;
  }
