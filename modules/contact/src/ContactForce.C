#include "ContactForce.h"

#include <cmath>

template<>
InputParameters validParams<ContactForce>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addRequiredParam<unsigned int>("component", "The direction this bc should act in.");
  
  params.addParam<Real>("penalty", 1e6, "The multiplier for the penalty.");

  params.addRequiredCoupledVar("penetration", "The amount of penetration this side has into the contacting surface.");
  
  return params;
}

ContactForce::ContactForce(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),
   _penetration(coupledValue("penetration")),
   _penalty(getParam<Real>("penalty")),
   _component(getParam<unsigned int>("component")),
   _youngs_modulus(getMaterialProperty<Real>("youngs_modulus"))
{}


Real
ContactForce::computeQpResidual()
{
  Real res = 0;

  res = _penalty*_youngs_modulus[_qp]*_penetration[_qp]*_phi[_i][_qp]*_normals[_qp](_component);

  return res;
}

Real
ContactForce::computeQpJacobian()
{
  Real res = 0;

  res = _penalty*_youngs_modulus[_qp]*_phi[_j][_qp]*_phi[_i][_qp]*_normals[_qp](_component);

  return res;
}
