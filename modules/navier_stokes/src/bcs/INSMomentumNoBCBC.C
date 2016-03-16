/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "INSMomentumNoBCBC.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<INSMomentumNoBCBC>()
{
  InputParameters params = validParams<IntegratedBC>();

  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", "z-velocity"); // only required in 3D
  params.addRequiredCoupledVar("p", "pressure");

  // Required parameters
  params.addRequiredParam<Real>("mu", "dynamic viscosity");
  params.addRequiredParam<Real>("rho", "density");
  params.addRequiredParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  params.addRequiredParam<unsigned>("component", "0,1,2 depending on if we are solving the x,y,z component of the momentum equation");
  params.addParam<bool>("integrate_p_by_parts", true, "Allows simulations to be run with pressure BC if set to false");

  return params;
}



INSMomentumNoBCBC::INSMomentumNoBCBC(const InputParameters & parameters) :
  IntegratedBC(parameters),

  // Coupled variables
  _u_vel(coupledValue("u")),
  _v_vel(_mesh.dimension() >= 2 ? coupledValue("v") : _zero),
  _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),
  _p(coupledValue("p")),

  // Gradients
  _grad_u_vel(coupledGradient("u")),
  _grad_v_vel(_mesh.dimension() >= 2 ? coupledGradient("v") : _grad_zero),
  _grad_w_vel(_mesh.dimension() == 3 ? coupledGradient("w") : _grad_zero),

  // Variable numberings
  _u_vel_var_number(coupled("u")),
  _v_vel_var_number(_mesh.dimension() >= 2 ? coupled("v") : libMesh::invalid_uint),
  _w_vel_var_number(_mesh.dimension() == 3 ? coupled("w") : libMesh::invalid_uint),
  _p_var_number(coupled("p")),

  // Required parameters
  _mu(getParam<Real>("mu")),
  _rho(getParam<Real>("rho")),
  _gravity(getParam<RealVectorValue>("gravity")),
  _component(getParam<unsigned>("component")),
  _integrate_p_by_parts(getParam<bool>("integrate_p_by_parts"))
{
}



Real INSMomentumNoBCBC::computeQpResidual()
{
  // Compute n . sigma . v, where n is unit normal and v is the test function.
  RealTensorValue sigma;

  // First row
  sigma(0,0) = 2.*_mu*_grad_u_vel[_qp](0);
  sigma(0,1) = _mu*(_grad_u_vel[_qp](1) + _grad_v_vel[_qp](0));
  sigma(0,2) = _mu*(_grad_u_vel[_qp](2) + _grad_w_vel[_qp](0));

  // Second row
  sigma(1,0) = _mu*(_grad_v_vel[_qp](0) + _grad_u_vel[_qp](1));
  sigma(1,1) = 2.*_mu*_grad_v_vel[_qp](1);
  sigma(1,2) = _mu*(_grad_v_vel[_qp](2) + _grad_w_vel[_qp](1));

  // Third row
  sigma(2,0) = _mu*(_grad_w_vel[_qp](0) + _grad_u_vel[_qp](2));
  sigma(2,1) = _mu*(_grad_w_vel[_qp](1) + _grad_v_vel[_qp](2));
  sigma(2,2) = 2.*_mu*_grad_w_vel[_qp](2);

  // If the pressure term is integrated by parts, it is part of the
  // no-BC-BC, otherwise, it is not.
  if (_integrate_p_by_parts)
  {
    sigma(0,0) -= _p[_qp];
    sigma(1,1) -= _p[_qp];
    sigma(2,2) -= _p[_qp];
  }

  // Set up test function
  RealVectorValue test;
  test(_component) = _test[_i][_qp];

  return _normals[_qp] * (sigma * test);
}




Real INSMomentumNoBCBC::computeQpJacobian()
{
  // The extra contribution comes from the "2" on the diagonal of the viscous stress tensor
  return _mu * (_grad_phi[_j][_qp]             * _normals[_qp] +
                _grad_phi[_j][_qp](_component) * _normals[_qp](_component)) * _test[_i][_qp];
}




Real INSMomentumNoBCBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_var_number)
    return _mu * _grad_phi[_j][_qp](_component) * _normals[_qp](0) * _test[_i][_qp];

  else if (jvar == _v_vel_var_number)
    return _mu * _grad_phi[_j][_qp](_component) * _normals[_qp](1) * _test[_i][_qp];

  else if (jvar == _w_vel_var_number)
    return _mu * _grad_phi[_j][_qp](_component) * _normals[_qp](2) * _test[_i][_qp];

  else if (jvar == _p_var_number)
  {
    if (_integrate_p_by_parts)
      return -_phi[_j][_qp] * _normals[_qp](_component) * _test[_i][_qp];
    else
      return 0.;
  }

  else
    return 0.;
}

