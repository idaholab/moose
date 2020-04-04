//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
// This was experimental code and did not really work out, do not use!
#include "NSEnergyInviscidSpecifiedDensityAndVelocityBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSEnergyInviscidSpecifiedDensityAndVelocityBC);

InputParameters
NSEnergyInviscidSpecifiedDensityAndVelocityBC::validParams()
{
  InputParameters params = NSEnergyInviscidBC::validParams();

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
