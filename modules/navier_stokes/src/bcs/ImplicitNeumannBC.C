#include "ImplicitNeumannBC.h"

template<>
InputParameters validParams<ImplicitNeumannBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  return params;
}



ImplicitNeumannBC::ImplicitNeumannBC(const std::string & name, InputParameters parameters) :
  IntegratedBC(name, parameters)
{
}



Real ImplicitNeumannBC::computeQpResidual()
{
  return _grad_u[_qp] * _normals[_qp] * _test[_i][_qp];
}




Real ImplicitNeumannBC::computeQpJacobian()
{
  return (_grad_phi[_j][_qp]*_normals[_qp]) * _test[_i][_qp];
}




Real ImplicitNeumannBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // off-diagonal derivatives are all zero.
  return 0.;
}
