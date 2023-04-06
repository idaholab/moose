//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVInletVelocityBC.h"
#include "INSFVVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVInletVelocityBC);

InputParameters
WCNSFVInletVelocityBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params += INSFVFlowBC::validParams();

  params.addParam<Real>("scaling_factor", 1, "To scale the velocity");

  // Two different ways to input velocity
  // 1) Postprocessor with the velocity value directly
  params.addParam<PostprocessorName>("velocity_pp", "Postprocessor with the inlet velocity");

  // 2) Postprocessors with an inlet mass flow rate
  params.addParam<PostprocessorName>("mdot_pp", "Postprocessor with the inlet mass flow rate");
  params.addParam<MooseFunctorName>(NS::density, "Density functor");
  params.addParam<PostprocessorName>("area_pp", "Inlet area as a postprocessor");

  return params;
}

WCNSFVInletVelocityBC::WCNSFVInletVelocityBC(const InputParameters & params)
  : FVDirichletBCBase(params),
    INSFVFlowBC(params),
    _scaling_factor(getParam<Real>("scaling_factor")),
    _velocity_pp(isParamValid("velocity_pp") ? &getPostprocessorValue("velocity_pp") : nullptr),
    _mdot_pp(isParamValid("mdot_pp") ? &getPostprocessorValue("mdot_pp") : nullptr),
    _area_pp(isParamValid("area_pp") ? &getPostprocessorValue("area_pp") : nullptr),
    _rho(isParamValid(NS::density) ? &getFunctor<ADReal>(NS::density) : nullptr)
{
  if (!dynamic_cast<INSFVVelocityVariable *>(&_var))
    paramError(
        "variable",
        "The variable argument to WCNSFVInletVelocityBC must be of type INSFVVelocityVariable");

  // Density is often set as global parameters so it is not checked
  if (_velocity_pp && (_mdot_pp || _area_pp))
    mooseWarning("If setting the velocity directly, no need for inlet mass flow rate or area");

  // Need enough information if trying to use a mass flow rate postprocessor
  if (!_velocity_pp && (!_mdot_pp || !_area_pp || !_rho))
    mooseError("Mass flow rate, area and density should be provided if velocity is not");
}

ADReal
WCNSFVInletVelocityBC::boundaryValue(const FaceInfo & fi) const
{
  if (_area_pp)
    if (MooseUtils::absoluteFuzzyEqual(*_area_pp, 0))
      mooseError("Surface area is 0");

  if (_velocity_pp)
    return _scaling_factor * (*_velocity_pp);
  else
  {
    ADReal rho = (*_rho)(singleSidedFaceArg(&fi), determineState());

    return _scaling_factor * (*_mdot_pp) / (*_area_pp * rho);
  }
}
