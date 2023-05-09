//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVMomentumFluxBC.h"
#include "INSFVVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVMomentumFluxBC);

InputParameters
WCNSFVMomentumFluxBC::validParams()
{
  InputParameters params = WCNSFVFluxBCBase::validParams();

  params += INSFVMomentumResidualObject::validParams();
  params.addClassDescription("Flux boundary conditions for momentum advection.");

  return params;
}

WCNSFVMomentumFluxBC::WCNSFVMomentumFluxBC(const InputParameters & params)
  : WCNSFVFluxBCBase(params), INSFVMomentumResidualObject(*this)
{
  if (!dynamic_cast<INSFVVelocityVariable *>(&_var))
    paramError(
        "variable",
        "The variable argument to WCNSFVMomentumFluxBC must be of type INSFVVelocityVariable");

  // Need enough information to compute the mass flux
  if (_mdot_pp && !_area_pp)
    mooseError("The inlet area should be provided along with the mass flow rate");
  if (!_mdot_pp && (!_velocity_pp || !_rho))
    mooseError("Velocity and density should be provided if the mass flow rate is not");
}

ADReal
WCNSFVMomentumFluxBC::computeQpResidual()
{
  if (_area_pp)
    if (MooseUtils::absoluteFuzzyEqual(*_area_pp, 0))
      mooseError("Surface area is 0");

  /*
   * We assume the following orientation: The supplied mass flow and velocity magnitude need to be
   * positive if:
   * 1. No direction parameter is supplied and we want to define an inlet condition (similarly, if
   * the mass flow/velocity magnitude are negative we define an outlet)
   * 2. If the fluid flows aligns with the direction parameter specified by the user. (similarly, if
   * the postprocessor values ae)
   *
   */
  checkForInternalDirection();
  const Point incoming_vector =
      !_direction_specified_by_user ? Point(-_face_info->normal()) : _direction;
  const Real cos_angle = std::abs(incoming_vector * _face_info->normal());

  if (_velocity_pp)
  {
    // In the case when the stream comes with an angle we need to multiply this quantity
    // with the dot product of the surface normal and the incoming jet direction
    return -_scaling_factor * std::pow((*_velocity_pp), 2) * incoming_vector(_index) * cos_angle *
           (*_rho)(singleSidedFaceArg(), determineState());
  }
  else
  // In this case the cosine of the angle is already incorporated in mdot so we will
  // have to correct back to get the right velocity magnitude
  {
    const auto velocity_magnitude =
        (*_mdot_pp) / ((*_area_pp) * (*_rho)(singleSidedFaceArg(), determineState()) * cos_angle);
    return -_scaling_factor * (*_mdot_pp) / (*_area_pp) * incoming_vector(_index) *
           velocity_magnitude;
  }
}
