#include "WeakGradientBC.h"

template<>
InputParameters validParams<WeakGradientBC>()
{
  InputParameters params;
  params.set<Real>("value")=0.0;
  return params;
}

WeakGradientBC::WeakGradientBC(std::string name, InputParameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(name, parameters, var_name, true, boundary_id, coupled_to, coupled_as),
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

