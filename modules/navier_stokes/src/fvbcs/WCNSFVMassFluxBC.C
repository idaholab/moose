//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVMassFluxBC.h"
#include "INSFVPressureVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVMassFluxBC);

InputParameters
WCNSFVMassFluxBC::validParams()
{
  InputParameters params = WCNSFVFluxBCBase::validParams();
  params.addClassDescription("Flux boundary conditions for mass advection.");
  params.addParam<Real>("scaling_factor", 1, "To scale the mass flux");
  return params;
}

WCNSFVMassFluxBC::WCNSFVMassFluxBC(const InputParameters & params) : WCNSFVFluxBCBase(params)
{
  if (!dynamic_cast<INSFVPressureVariable *>(&_var))
    paramError("variable",
               "The variable argument to WCNSFVMassFluxBC must be of type INSFVPressureVariable");

  // Need enough information to compute the mass flux
  if (_mdot_pp && !_area_pp)
    mooseError("The inlet area should be provided along with the mass flow rate");
  if (!_mdot_pp && !_velocity_pp)
    mooseError("Velocity should be provided if the mass flow rate is not");
}

ADReal
WCNSFVMassFluxBC::computeQpResidual()
{
  const auto state = determineState();

  if (!isInflow())
  {
    const auto fa = singleSidedFaceArg();
    return varVelocity(state) * _normal * _rho(fa, state);
  }

  return -_scaling_factor * inflowMassFlux(state);
}
