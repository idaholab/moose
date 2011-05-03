#include "Convection.h"

template<>
InputParameters validParams<Convection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<Real>("x", "Component of velocity in the x direction");
  params.addRequiredParam<Real>("y", "Component of velocity in the y direction");
  params.addParam<Real>("z", 0.0, "Component of velocity in the z direction");
  return params;
}

Convection::Convection(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _x(getParam<Real>("x")),
    _y(getParam<Real>("y")),
    _z(getParam<Real>("z"))
{
  _velocity(0) = _x;
  _velocity(1) = _y;
  _velocity(2) = _z;
}

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
