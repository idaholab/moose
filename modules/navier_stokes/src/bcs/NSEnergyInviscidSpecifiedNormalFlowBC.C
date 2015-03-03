/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSEnergyInviscidSpecifiedNormalFlowBC.h"

template<>
InputParameters validParams<NSEnergyInviscidSpecifiedNormalFlowBC>()
{
  InputParameters params = validParams<NSEnergyInviscidBC>();

  // Coupled variables
  params.addRequiredCoupledVar("pressure", "");

  // Required parameters
  params.addRequiredParam<Real>("un", "The specified value of u.n for this boundary");

  return params;
}



NSEnergyInviscidSpecifiedNormalFlowBC::NSEnergyInviscidSpecifiedNormalFlowBC(const std::string & name, InputParameters parameters)
    : NSEnergyInviscidBC(name, parameters),

      // Aux Variables
      _pressure(coupledValue("pressure")),

      // Required parameters
      _un(getParam<Real>("un"))
{
}




Real NSEnergyInviscidSpecifiedNormalFlowBC::computeQpResidual()
{
  return this->qp_residual(_pressure[_qp], _un);
}




Real NSEnergyInviscidSpecifiedNormalFlowBC::computeQpJacobian()
{
  return this->compute_jacobian(/*on-diagonal variable is energy=*/4);
}




Real NSEnergyInviscidSpecifiedNormalFlowBC::computeQpOffDiagJacobian(unsigned jvar)
{
  return this->compute_jacobian( this->map_var_number(jvar) );
}




Real NSEnergyInviscidSpecifiedNormalFlowBC::compute_jacobian(unsigned var_number)
{
  // For specified u.n, term "A" is zero, see base class for details.
  return
    this->qp_jacobian_termB(var_number, _un) +
    this->qp_jacobian_termC(var_number, _un);
}
