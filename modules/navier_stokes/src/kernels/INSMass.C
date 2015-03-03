/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMass.h"

template<>
InputParameters validParams<INSMass>()
{
  InputParameters params = validParams<Kernel>();

  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", "z-velocity"); // only required in 3D
  params.addRequiredCoupledVar("p", "pressure");

  return params;
}



INSMass::INSMass(const std::string & name, InputParameters parameters) :
  Kernel(name, parameters),

  // Gradients
  _grad_u_vel(coupledGradient("u")),
  _grad_v_vel(_mesh.dimension() >= 2 ? coupledGradient("v") : _grad_zero),
  _grad_w_vel(_mesh.dimension() == 3 ? coupledGradient("w") : _grad_zero),

  // Variable numberings
  _u_vel_var_number(coupled("u")),
  _v_vel_var_number(_mesh.dimension() >= 2 ? coupled("v") : libMesh::invalid_uint),
  _w_vel_var_number(_mesh.dimension() == 3 ? coupled("w") : libMesh::invalid_uint),
  _p_var_number(coupled("p"))
{
}



Real INSMass::computeQpResidual()
{
  // (div u) * q
  // Note: we (arbitrarily) multilply this term by -1 so that it matches the -p(div v)
  // term in the momentum equation.  Not sure if that is really important?
  return -(_grad_u_vel[_qp](0) + _grad_v_vel[_qp](1) + _grad_w_vel[_qp](2)) * _test[_i][_qp];
}




Real INSMass::computeQpJacobian()
{
  // Derivative wrt to p is zero
  return 0;
}




Real INSMass::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_var_number)
    return -_grad_phi[_j][_qp](0) * _test[_i][_qp];

  else if (jvar == _v_vel_var_number)
    return -_grad_phi[_j][_qp](1) * _test[_i][_qp];

  else if (jvar == _w_vel_var_number)
    return -_grad_phi[_j][_qp](2) * _test[_i][_qp];
  else
    return 0;
}
