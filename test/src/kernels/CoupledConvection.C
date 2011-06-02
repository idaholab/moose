#include "CoupledConvection.h"

template<>
InputParameters validParams<CoupledConvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("velocity_vector", "Velocity Vector for the Convection Kernel");
  return params;
}

CoupledConvection::CoupledConvection(const std::string & name, InputParameters parameters) :
  Kernel(name, parameters),
  _velocity_vector(coupledGradient("velocity_vector"))
{}

Real
CoupledConvection::computeQpResidual()
{
  return _test[_i][_qp]*(_velocity_vector[_qp]*_grad_u[_qp]);
}

Real
CoupledConvection::computeQpJacobian()
{
  return _test[_i][_qp]*(_velocity_vector[_qp]*_grad_phi[_j][_qp]);
}
