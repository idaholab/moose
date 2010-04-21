#include "VacuumBC.h"

template<>
InputParameters validParams<VacuumBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<Real>("alpha")=1;
  return params;
}

VacuumBC::VacuumBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, setIntegratedParam(parameters, true)),
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
