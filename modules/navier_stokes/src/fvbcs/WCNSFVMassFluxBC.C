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
  if (!_mdot_pp && (!_velocity_pp || !_rho))
    mooseError("Velocity and density should be provided if the mass flow rate is not");
}

ADReal
WCNSFVMassFluxBC::computeQpResidual()
{
  if (_area_pp)
    if (MooseUtils::absoluteFuzzyEqual(*_area_pp, 0))
      mooseError("Surface area is 0");

  /*
   * We assume the following orientation: The supplied mass flow and velocity magnitude need to be
   * positive if:
   * 1. No direction parameter is supplied and we want to define an inlet condition (similarly, if
   * the mass flow/velocity magnitude are negative we define an outlet)
   * 2. If the fluid flows aligns with the direction parameter specified by the user. (similarly,
   * if the postprocessor values are negative we assume the fluid flows backwards with respect to
   * the direction parameter)
   */
  if (_velocity_pp)
  {
    checkForInternalDirection();
    const Point incoming_vector = !_direction_specified_by_user ? _face_info->normal() : _direction;
    const Real cos_angle = std::abs(incoming_vector * _face_info->normal());
    return -_scaling_factor * (*_velocity_pp) * cos_angle *
           (*_rho)(singleSidedFaceArg(), determineState());
  }
  else
    return -_scaling_factor * (*_mdot_pp) / (*_area_pp);
}
