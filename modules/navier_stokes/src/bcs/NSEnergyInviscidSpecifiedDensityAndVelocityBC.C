/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
// This was experimental code and did not really work out, do not use!
#include "NSEnergyInviscidSpecifiedDensityAndVelocityBC.h"

template<>
InputParameters validParams<NSEnergyInviscidSpecifiedDensityAndVelocityBC>()
{
  InputParameters params = validParams<NSEnergyInviscidBC>();

  // Coupled variables
  params.addRequiredCoupledVar("pressure", "");

  // Required parameters
  params.addRequiredParam<Real>("specified_density", "The specified density for this boundary");
  params.addRequiredParam<Real>("specified_u", "The x-component of the specified velocity for this boundary");
  params.addRequiredParam<Real>("specified_v", "The y-component of the specified velocity for this boundary");
  params.addParam<Real>("specified_w", 0., "The z-component of the specified velocity for this boundary"); // only required in 3D

  return params;
}



NSEnergyInviscidSpecifiedDensityAndVelocityBC::NSEnergyInviscidSpecifiedDensityAndVelocityBC(const std::string & name, InputParameters parameters)
    : NSEnergyInviscidBC(name, parameters),

      // Aux Variables
      _pressure(coupledValue("pressure")),

      // Required parameters
      _specified_density(getParam<Real>("specified_density")),
      _specified_u(getParam<Real>("specified_u")),
      _specified_v(getParam<Real>("specified_v")),
      _specified_w(getParam<Real>("specified_w"))
{
}




Real NSEnergyInviscidSpecifiedDensityAndVelocityBC::computeQpResidual()
{
  return this->qp_residual(_specified_density,
                           RealVectorValue(_specified_u, _specified_v, _specified_w),
                           _pressure[_qp]);
}




Real NSEnergyInviscidSpecifiedDensityAndVelocityBC::computeQpJacobian()
{
  // TODO
  // return this->compute_jacobian(/*on-diagonal variable is energy=*/4);
  return 0.;
}




Real NSEnergyInviscidSpecifiedDensityAndVelocityBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // TODO
  // return this->compute_jacobian( this->map_var_number(jvar) );
  return 0.;
}




// Real NSEnergyInviscidSpecifiedDensityAndVelocityBC::compute_jacobian(unsigned var_number)
// {
//   // When both pressure and u.n are specified, only term B of the Jacobian is non-zero.
//   return this->qp_jacobian_termB(var_number, _un);
// }
