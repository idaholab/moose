//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVFluxBCBase.h"
#include "NS.h"

InputParameters
WCNSFVFluxBCBase::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params += INSFVFlowBC::validParams();
  params.addParam<Real>("scaling_factor", 1, "To scale the mass flux");

  params.addParam<PostprocessorName>("velocity_pp", "Postprocessor with the inlet velocity norm");
  params.addParam<Point>(
      "direction",
      Point(),
      "The direction of the flow at the boundary. This is mainly used for cases when an inlet "
      "angle needs to be defined with respect to the normal and when a boundary is defined on an "
      "internal face where the normal can point in both directions. Use positive mass flux and "
      "velocity magnitude if the flux aligns with this direction vector.");
  params.addRequiredParam<MooseFunctorName>(NS::velocity_x, "The x-axis velocity");
  params.addParam<MooseFunctorName>(NS::velocity_y, "The y-axis velocity");
  params.addParam<MooseFunctorName>(NS::velocity_z, "The z-axis velocity");
  params.addParam<PostprocessorName>("mdot_pp", "Postprocessor with the inlet mass flow rate");
  params.addParam<PostprocessorName>("area_pp", "Inlet area as a postprocessor");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density functor");

  return params;
}

WCNSFVFluxBCBase::WCNSFVFluxBCBase(const InputParameters & params)
  : FVFluxBC(params),
    INSFVFlowBC(params),
    _scaling_factor(getParam<Real>("scaling_factor")),
    _velocity_pp(isParamValid("velocity_pp") ? &getPostprocessorValue("velocity_pp") : nullptr),
    _mdot_pp(isParamValid("mdot_pp") ? &getPostprocessorValue("mdot_pp") : nullptr),
    _area_pp(isParamValid("area_pp") ? &getPostprocessorValue("area_pp") : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _direction(getParam<Point>("direction")),
    _direction_specified_by_user(params.isParamSetByUser("direction")),
    _vel_x(getFunctor<ADReal>(NS::velocity_x)),
    _vel_y(isParamValid(NS::velocity_y) ? &getFunctor<ADReal>(NS::velocity_y) : nullptr),
    _vel_z(isParamValid(NS::velocity_z) ? &getFunctor<ADReal>(NS::velocity_z) : nullptr)
{
  if (_direction_specified_by_user && !MooseUtils::absoluteFuzzyEqual(_direction.norm(), 1.0, 1e-6))
    paramError("direction", "The direction should be a unit vector with a tolerance of 1e-6!");

  // Density is often set as global parameters so it is not checked
  if (_mdot_pp && _velocity_pp)
    mooseWarning("If setting the mass flow rate directly, no need for inlet velocity");

  if (_subproblem.mesh().dimension() >= 2 && !_vel_y)
    mooseError("In two or more dimensions, the y-component of the velocity must be supplied.");
  if (_subproblem.mesh().dimension() == 3 && !_vel_z)
    mooseError("In three dimensions, the z-component of the velocity must be supplied.");
}

// inflow is decided based on the sign of mdot or velocity postprocessors
// depending on what is provided
bool
WCNSFVFluxBCBase::isInflow() const
{
  if (_mdot_pp)
    return *_mdot_pp >= 0;
  else if (_velocity_pp)
    return *_velocity_pp >= 0;

  mooseError("Either mdot_pp or velocity_pp need to be provided OR this function must be "
             "overridden in derived classes if other input parameter combinations are valid. "
             "Neither mdot_pp nor velocity_pp are provided.");
  return true;
}

ADRealVectorValue
WCNSFVFluxBCBase::varVelocity(const Moose::StateArg & state) const
{
  const auto boundary_face = singleSidedFaceArg();

  ADRealVectorValue v(_vel_x(boundary_face, state));
  if (_vel_y)
    v(1) = (*_vel_y)(boundary_face, state);
  if (_vel_z)
    v(2) = (*_vel_z)(boundary_face, state);
  return v;
}

ADReal
WCNSFVFluxBCBase::inflowMassFlux(const Moose::StateArg & state) const
{
  checkForInternalDirection();
  if (_mdot_pp)
    return *_mdot_pp / *_area_pp;
  const ADRealVectorValue incoming_vector = !_direction_specified_by_user ? _normal : _direction;
  const ADReal cos_angle = std::abs(incoming_vector * _normal);
  return _rho(singleSidedFaceArg(), state) * (*_velocity_pp) * cos_angle;
}

ADReal
WCNSFVFluxBCBase::inflowSpeed(const Moose::StateArg & state) const
{
  checkForInternalDirection();
  const ADRealVectorValue incoming_vector = !_direction_specified_by_user ? _normal : _direction;
  const ADReal cos_angle = std::abs(incoming_vector * _normal);
  if (_mdot_pp)
    return *_mdot_pp / (*_area_pp * _rho(singleSidedFaceArg(), state) * cos_angle);

  return (*_velocity_pp) * cos_angle;
}

void
WCNSFVFluxBCBase::residualSetup()
{
  if (_area_pp)
    if (MooseUtils::absoluteFuzzyEqual(*_area_pp, 0))
      mooseError("Surface area is 0");
  FVFluxBC::residualSetup();
}

void
WCNSFVFluxBCBase::jacobianSetup()
{
  if (_area_pp)
    if (MooseUtils::absoluteFuzzyEqual(*_area_pp, 0))
      mooseError("Surface area is 0");
  FVFluxBC::jacobianSetup();
}
