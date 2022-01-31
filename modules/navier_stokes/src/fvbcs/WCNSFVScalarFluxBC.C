//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVScalarFluxBC.h"
#include "INSFVEnergyVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVScalarFluxBC);

InputParameters
WCNSFVScalarFluxBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params += INSFVFlowBC::validParams();
  params.addClassDescription("Flux boundary conditions for scalar quantity advection.");

  params.addParam<Real>("scaling_factor", 1, "To scale the flux");

  // Three different ways to input an advected scalar flux:
  // 1) Postprocessor with the scalar flow rate directly
  params.addParam<PostprocessorName>("scalar_flux_pp",
                                     "Postprocessor with the inlet scalar flow rate");
  params.addParam<PostprocessorName>("area_pp", "Postprocessor with the inlet flow area");

  // 2) Postprocessors for inlet velocity and scalar concentration
  params.addParam<PostprocessorName>("scalar_value_pp",
                                     "Postprocessor with the inlet scalar concentration");
  params.addParam<PostprocessorName>("velocity_pp", "Postprocessor with the inlet velocity norm");

  // 3) Postprocessors for mass flow rate and energy, functor for density
  params.addParam<PostprocessorName>("mdot_pp", "Postprocessor with the inlet mass flow rate");
  params.addParam<MooseFunctorName>(NS::density, "Density functor");

  return params;
}

WCNSFVScalarFluxBC::WCNSFVScalarFluxBC(const InputParameters & params)
  : FVFluxBC(params),
    INSFVFlowBC(params),
    _scaling_factor(getParam<Real>("scaling_factor")),
    _scalar_value_pp(isParamValid("scalar_value_pp") ? &getPostprocessorValue("scalar_value_pp")
                                                     : nullptr),
    _scalar_flux_pp(isParamValid("scalar_flux_pp") ? &getPostprocessorValue("scalar_flux_pp")
                                                   : nullptr),
    _velocity_pp(isParamValid("velocity_pp") ? &getPostprocessorValue("velocity_pp") : nullptr),
    _mdot_pp(isParamValid("mdot_pp") ? &getPostprocessorValue("mdot_pp") : nullptr),
    _area_pp(isParamValid("area_pp") ? &getPostprocessorValue("area_pp") : nullptr),
    _rho(isParamValid(NS::density) ? &getFunctor<ADReal>(NS::density) : nullptr)
{
  // Density is often set as global parameters so it is not checked
  if (_scalar_flux_pp && (_velocity_pp || _mdot_pp || _scalar_value_pp))
    mooseWarning(
        "If setting the scalar flux directly, no need for inlet velocity, mass flow or scalar "
        "concentration");

  // Need enough information if trying to use a mass flow rate postprocessor
  if (!_scalar_flux_pp)
  {
    if (!_scalar_value_pp)
      mooseError("If not providing the scalar flow rate, the inlet scalar concentration should be "
                 "provided");
    if (!_velocity_pp && !_mdot_pp)
      mooseError("If not providing the scalar flow rate, the inlet velocity or mass flow "
                 "should be provided");
    if (_mdot_pp && (!_rho || !_area_pp))
      mooseError("If providing the inlet mass flow rate, the inlet density and flow "
                 "area should be provided as well");
  }
  else if (!_area_pp)
    paramError("scalar_flux_pp",
               "If supplying the energy flow rate, the flow area should be provided as well");
}

ADReal
WCNSFVScalarFluxBC::computeQpResidual()
{
  if (_area_pp)
    if (MooseUtils::absoluteFuzzyEqual(*_area_pp, 0))
      mooseError("Surface area is 0");

  if (_scalar_flux_pp)
    return -_scaling_factor * *_scalar_flux_pp / *_area_pp;
  else if (_velocity_pp)
    return -_scaling_factor * *_velocity_pp * *_scalar_value_pp;
  else
    return -_scaling_factor * *_mdot_pp / *_area_pp / (*_rho)(singleSidedFaceArg()) *
           *_scalar_value_pp;
}
