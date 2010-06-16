#include "VacuumBC.h"

template<>
InputParameters validParams<VacuumBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addParam<Real>("alpha", 1, "No idea.");
  return params;
}

VacuumBC::VacuumBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),
    _alpha(_parameters.get<Real>("alpha"))
  {}

Real
VacuumBC::computeQpResidual()
  {
    return _phi[_i][_qp]*_alpha*_u[_qp]/2.;
  }

Real
VacuumBC::computeQpJacobian()
  {
    return _phi[_i][_qp]*_alpha*_phi[_j][_qp]/2.;    
  }
