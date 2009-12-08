#include "DirichletBC.h"

template<>
InputParameters validParams<DirichletBC>()
{
  InputParameters params;
  params.set<Real>("value")=0.0;
  return params;
}

DirichletBC::DirichletBC(std::string name, InputParameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(name, parameters, var_name, false, boundary_id, coupled_to, coupled_as),
    _value(_parameters.get<Real>("value"))
  {}

Real
DirichletBC::computeQpResidual()
  {
    return _u_face[_qp]-_value;
  }
