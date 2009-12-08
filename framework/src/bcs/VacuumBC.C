#include "VacuumBC.h"

template<>
InputParameters validParams<VacuumBC>()
{
  InputParameters params;
  params.set<Real>("alpha")=1;
  return params;
}

VacuumBC::VacuumBC(std::string name, InputParameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(name, parameters, var_name, true, boundary_id, coupled_to, coupled_as),
    _alpha(_parameters.get<Real>("alpha"))
  {}

Real
VacuumBC::computeQpResidual()
  {
    return _phi_face[_i][_qp]*_alpha*_u_face[_qp]/2.;
  }

Real
VacuumBC::computeQpJacobian()
  {
    return _phi_face[_i][_qp]*_alpha*_phi_face[_j][_qp]/2.;    
  }
