/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSEnergyInviscidUnspecifiedBC.h"

template<>
InputParameters validParams<NSEnergyInviscidUnspecifiedBC>()
{
  InputParameters params = validParams<NSEnergyInviscidBC>();

  // Coupled variables
  params.addRequiredCoupledVar("pressure", "");

  return params;
}



NSEnergyInviscidUnspecifiedBC::NSEnergyInviscidUnspecifiedBC(const std::string & name, InputParameters parameters)
    : NSEnergyInviscidBC(name, parameters),

      // Aux Variables
      _pressure(coupledValue("pressure"))
{
}




Real NSEnergyInviscidUnspecifiedBC::computeQpResidual()
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Normal component
  Real un = vel * _normals[_qp];

  return this->qp_residual(_pressure[_qp], un);
}




Real NSEnergyInviscidUnspecifiedBC::computeQpJacobian()
{
  return this->compute_jacobian(/*on-diagonal variable is energy=*/4);
}




Real NSEnergyInviscidUnspecifiedBC::computeQpOffDiagJacobian(unsigned jvar)
{
  return this->compute_jacobian( this->map_var_number(jvar) );
}




Real NSEnergyInviscidUnspecifiedBC::compute_jacobian(unsigned var_number)
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Normal component of velocity
  Real un = vel * _normals[_qp];

  // When both u.n and pressure are unspecified, all 3 Jacobian terms apply.
  // See base class for details.
  return
    this->qp_jacobian_termA(var_number, _pressure[_qp]) +
    this->qp_jacobian_termB(var_number, un) +
    this->qp_jacobian_termC(var_number, un);
}
