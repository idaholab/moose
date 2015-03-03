/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSEnergyInviscidSpecifiedPressureBC.h"

template<>
InputParameters validParams<NSEnergyInviscidSpecifiedPressureBC>()
{
  InputParameters params = validParams<NSEnergyInviscidBC>();

  // Required parameters
  params.addRequiredParam<Real>("specified_pressure", "The specified pressure for this boundary");

  return params;
}



NSEnergyInviscidSpecifiedPressureBC::NSEnergyInviscidSpecifiedPressureBC(const std::string & name, InputParameters parameters)
    : NSEnergyInviscidBC(name, parameters),

      // Required parameters
     _specified_pressure(getParam<Real>("specified_pressure"))
{
}




Real NSEnergyInviscidSpecifiedPressureBC::computeQpResidual()
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Normal component
  Real un = vel * _normals[_qp];

  return this->qp_residual(_specified_pressure, un);
}




Real NSEnergyInviscidSpecifiedPressureBC::computeQpJacobian()
{
  return this->compute_jacobian(/*on-diagonal variable is energy=*/4);
}




Real NSEnergyInviscidSpecifiedPressureBC::computeQpOffDiagJacobian(unsigned jvar)
{
  return this->compute_jacobian( this->map_var_number(jvar) );
}




Real NSEnergyInviscidSpecifiedPressureBC::compute_jacobian(unsigned var_number)
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Normal component of velocity
  Real un = vel * _normals[_qp];

  // For specified pressure, term "C" is zero, see base class for details.
  return
    this->qp_jacobian_termA(var_number, _specified_pressure) +
    this->qp_jacobian_termB(var_number, un);
}
