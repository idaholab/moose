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
  InputParameters params = FVFluxBC::validParams();

  // this is not really necessary, since advection kernel wont be executed on
  // this flux boundary
  params += INSFVFlowBC::validParams();
  params.addClassDescription("Flux boundary conditions for mass advection.");

  params.addParam<Real>("scaling_factor", 1, "To scale the mass flux");

  // Two different ways to input velocity
  // 1) Postprocessor with the velocity value
  params.addParam<PostprocessorName>("velocity_pp", "Postprocessor with the inlet velocity norm");
  params.addParam<Point>(
      "direction",
      Point(),
      "The direction of the flow at the boundary. This is mainly used for cases when an inlet "
      "angle needs to be defined with respect to the normal and when a boundary is defined on an "
      "internal face where the normal can point in both directions. Use positive mass flux and "
      "velocity magnitude if the flux aligns with this direction vector.");
  params.addParam<MooseFunctorName>(NS::density, "Density functor");

  // 2) Postprocessors with an inlet mass flow rate directly
  params.addParam<PostprocessorName>("mdot_pp", "Postprocessor with the inlet mass flow rate");
  params.addParam<PostprocessorName>("area_pp", "Inlet area as a postprocessor");

  return params;
}

WCNSFVMassFluxBC::WCNSFVMassFluxBC(const InputParameters & params)
  : FVFluxBC(params),
    INSFVFlowBC(params),
    _scaling_factor(getParam<Real>("scaling_factor")),
    _velocity_pp(isParamValid("velocity_pp") ? &getPostprocessorValue("velocity_pp") : nullptr),
    _mdot_pp(isParamValid("mdot_pp") ? &getPostprocessorValue("mdot_pp") : nullptr),
    _area_pp(isParamValid("area_pp") ? &getPostprocessorValue("area_pp") : nullptr),
    _rho(isParamValid(NS::density) ? &getFunctor<ADReal>(NS::density) : nullptr),
    _direction(getParam<Point>("direction")),
    _direction_specified_by_user(params.isParamSetByUser("direction"))
{
  if (_direction_specified_by_user && !MooseUtils::absoluteFuzzyEqual(_direction.norm(), 1.0, 1e-6))
    paramError("direction", "The direction should be a unit vector with a tolerance of 1e-6!");

  if (!dynamic_cast<INSFVPressureVariable *>(&_var))
    paramError("variable",
               "The variable argument to WCNSFVMassFluxBC must be of type INSFVPressureVariable");

  // Density is often set as global parameters so it is not checked
  if (_mdot_pp && _velocity_pp)
    mooseWarning("If setting the mass flow rate directly, no need for inlet velocity");

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
    if (_face_info->neighborPtr() && !_direction_specified_by_user)
      paramError("direction",
                 this->type(),
                 " can only be defined on an internal face if a direction parameter is supplied!");
    const Point incoming_vector = !_direction_specified_by_user ? _face_info->normal() : _direction;
    const Real cos_angle = std::abs(incoming_vector * _face_info->normal());
    return -_scaling_factor * (*_velocity_pp) * cos_angle * (*_rho)(singleSidedFaceArg());
  }
  else
    return -_scaling_factor * (*_mdot_pp) / (*_area_pp);
}
