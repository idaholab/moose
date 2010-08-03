#include "Convection.h"

template<>
InputParameters validParams<Convection>()
{
  InputParameters params = validParams<Kernel>();

  params.addRequiredCoupledVar("velocity_vector", "Velocity Vector for the Convection Kernel");
  return params;
}

Convection::Convection(const std::string & name,
                       MooseSystem &sys,
                       InputParameters & parameters)

  :Kernel(name, sys, parameters),
   _velocity_vector(coupledGradient("velocity_vector"))
{}

Real Convection::computeQpResidual()
{
 return _test[_i][_qp]*(_velocity_vector[_qp]*_grad_u[_qp]);
}

Real Convection::computeQpJacobian()
{
  return _test[_i][_qp]*(_velocity_vector[_qp]*_grad_phi[_j][_qp]);
}
