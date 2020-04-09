//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSInflowThermalBC.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

registerMooseObject("NavierStokesApp", NSInflowThermalBC);

InputParameters
NSInflowThermalBC::validParams()
{
  InputParameters params = NodalBC::validParams();

  params.addClassDescription("This class is used on a boundary where the incoming flow values "
                             "(rho, u, v, T) are all completely specified.");
  // Boundary condition values, all required except for velocity which defaults to zero.
  params.addRequiredParam<Real>("specified_rho", "Density of incoming flow");
  params.addRequiredParam<Real>("specified_temperature", "Temperature of incoming flow");
  params.addParam<Real>("specified_velocity_magnitude", 0., "Velocity magnitude of incoming flow");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  return params;
}

NSInflowThermalBC::NSInflowThermalBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _specified_rho(getParam<Real>("specified_rho")),
    _specified_temperature(getParam<Real>("specified_temperature")),
    _specified_velocity_magnitude(getParam<Real>("specified_velocity_magnitude")),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
}

Real
NSInflowThermalBC::computeQpResidual()
{
  // For the total energy, the essential BC is:
  // rho*E = rho*(c_v*T + 0.5*|u|^2)
  //
  // or, in residual form, (In general, this BC is coupled to the velocity variables.)
  // rho*E - rho*(c_v*T + 0.5*|u|^2) = 0
  //
  // ***at a no-slip wall*** this further reduces to (no coupling to velocity variables):
  // rho*E - rho*cv*T = 0
  return _u[_qp] -
         _specified_rho * (_fp.cv() * _specified_temperature +
                           0.5 * _specified_velocity_magnitude * _specified_velocity_magnitude);
}
