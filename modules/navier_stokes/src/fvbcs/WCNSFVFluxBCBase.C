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

  params.addParam<PostprocessorName>("mdot_pp", "Postprocessor with the inlet mass flow rate");
  params.addParam<PostprocessorName>("area_pp", "Inlet area as a postprocessor");
  params.addParam<MooseFunctorName>(NS::density, "Density functor");

  return params;
}

WCNSFVFluxBCBase::WCNSFVFluxBCBase(const InputParameters & params)
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

  // Density is often set as global parameters so it is not checked
  if (_mdot_pp && _velocity_pp)
    mooseWarning("If setting the mass flow rate directly, no need for inlet velocity");
}
