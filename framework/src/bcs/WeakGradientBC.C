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
    return (_grad_u_face[_qp]*_normals_face[_qp])*_phi_face[_i][_qp];
  }

Real
WeakGradientBC::computeQpJacobian()
  {
    return (_dphi_face[_j][_qp]*_normals_face[_qp])*_phi_face[_i][_qp];
  }

