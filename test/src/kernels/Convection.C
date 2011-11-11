#include "Convection.h"

template<>
InputParameters validParams<Convection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity Vector");
  return params;
}

Convection::Convection(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _velocity(getParam<RealVectorValue>("velocity"))
{}

Real
Convection::computeQpResidual()
{
  return _test[_i][_qp]*(_velocity*_grad_u[_qp]);
}

Real
Convection::computeQpJacobian()
{
  return _test[_i][_qp]*(_velocity*_grad_phi[_j][_qp]);
}

