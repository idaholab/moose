/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSEnergyInviscidSpecifiedBC.h"

template<>
InputParameters validParams<NSEnergyInviscidSpecifiedBC>()
{
  InputParameters params = validParams<NSEnergyInviscidBC>();

  // Required parameters
  params.addRequiredParam<Real>("specified_pressure", "The specified pressure for this boundary");
  params.addRequiredParam<Real>("un", "The specified value of u.n for this boundary");

  return params;
}



NSEnergyInviscidSpecifiedBC::NSEnergyInviscidSpecifiedBC(const std::string & name, InputParameters parameters)
    : NSEnergyInviscidBC(name, parameters),

      // Required parameters
      _specified_pressure(getParam<Real>("specified_pressure")),
      _un(getParam<Real>("un"))
{
}




Real NSEnergyInviscidSpecifiedBC::computeQpResidual()
{
  return this->qp_residual(_specified_pressure, _un);
}




Real NSEnergyInviscidSpecifiedBC::computeQpJacobian()
{
  return this->compute_jacobian(/*on-diagonal variable is energy=*/4);
}




Real NSEnergyInviscidSpecifiedBC::computeQpOffDiagJacobian(unsigned jvar)
{
  return this->compute_jacobian( this->map_var_number(jvar) );
}




Real NSEnergyInviscidSpecifiedBC::compute_jacobian(unsigned var_number)
{
  // When both pressure and u.n are specified, only term B of the Jacobian is non-zero.
  return this->qp_jacobian_termB(var_number, _un);
}
