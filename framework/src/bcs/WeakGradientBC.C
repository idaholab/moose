#include "WeakGradientBC.h"

template<>
InputParameters validParams<WeakGradientBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<Real>("value")=0.0;
  return params;
}

WeakGradientBC::WeakGradientBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, setIntegratedParam(parameters, true)),
    _value(_parameters.get<Real>("value"))
 {}

Real
WeakGradientBC::computeQpResidual()
  {
    return (_grad_u[_qp]*_normals[_qp])*_phi[_i][_qp];
  }

Real
WeakGradientBC::computeQpJacobian()
  {
    return (_grad_phi[_j][_qp]*_normals[_qp])*_phi[_i][_qp];
  }

