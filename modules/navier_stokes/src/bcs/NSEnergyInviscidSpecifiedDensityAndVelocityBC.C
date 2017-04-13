/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
// This was experimental code and did not really work out, do not use!
#include "NSEnergyInviscidSpecifiedDensityAndVelocityBC.h"
#include "NS.h"

template <>
InputParameters
validParams<NSEnergyInviscidSpecifiedDensityAndVelocityBC>()
{
  InputParameters params = validParams<NSEnergyInviscidBC>();

  // Coupled variables
  params.addRequiredCoupledVar(NS::pressure, "pressure");

  // Required parameters
  params.addRequiredParam<Real>("specified_density", "The specified density for this boundary");
  params.addRequiredParam<Real>("specified_u",
                                "The x-component of the specified velocity for this boundary");
  params.addRequiredParam<Real>("specified_v",
                                "The y-component of the specified velocity for this boundary");
  params.addParam<Real>(
      "specified_w",
      0.0,
      "The z-component of the specified velocity for this boundary"); // only required in 3D

  return params;
}

NSEnergyInviscidSpecifiedDensityAndVelocityBC::NSEnergyInviscidSpecifiedDensityAndVelocityBC(
    const InputParameters & parameters)
  : NSEnergyInviscidBC(parameters),
    _pressure(coupledValue(NS::pressure)),
    _specified_density(getParam<Real>("specified_density")),
    _specified_u(getParam<Real>("specified_u")),
    _specified_v(getParam<Real>("specified_v")),
    _specified_w(getParam<Real>("specified_w"))
{
}

Real
NSEnergyInviscidSpecifiedDensityAndVelocityBC::computeQpResidual()
{
  return qpResidualHelper(_specified_density,
                          RealVectorValue(_specified_u, _specified_v, _specified_w),
                          _pressure[_qp]);
}

Real
NSEnergyInviscidSpecifiedDensityAndVelocityBC::computeQpJacobian()
{
  // TODO
  // return computeJacobianHelper(/*on-diagonal variable is energy=*/4);
  return 0.;
}

Real
NSEnergyInviscidSpecifiedDensityAndVelocityBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // TODO
  // return computeJacobianHelper(mapVarNumber(jvar));
  return 0.;
}
