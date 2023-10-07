//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVScalarFluxBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVScalarFluxBC);

InputParameters
WCNSFVScalarFluxBC::validParams()
{
  InputParameters params = WCNSFVFluxBCBase::validParams();
  params.addClassDescription("Flux boundary conditions for scalar quantity advection.");

  // Two different ways to input an advected scalar flux:
  // 1) Postprocessor with the scalar flow rate directly
  // 2) Postprocessors for inlet velocity and scalar concentration
  params.addParam<PostprocessorName>("scalar_flux_pp",
                                     "Postprocessor with the inlet scalar flow rate");
  params.addParam<PostprocessorName>("scalar_value_pp",
                                     "Postprocessor with the inlet scalar concentration");
  params.addRequiredParam<MooseFunctorName>("passive_scalar", "passive scalar functor");
  return params;
}

WCNSFVScalarFluxBC::WCNSFVScalarFluxBC(const InputParameters & params)
  : WCNSFVFluxBCBase(params),
    _scalar_value_pp(isParamValid("scalar_value_pp") ? &getPostprocessorValue("scalar_value_pp")
                                                     : nullptr),
    _scalar_flux_pp(isParamValid("scalar_flux_pp") ? &getPostprocessorValue("scalar_flux_pp")
                                                   : nullptr),
    _passive_scalar(getFunctor<ADReal>("passive_scalar"))
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
    if (_mdot_pp && !_area_pp)
      mooseError("If providing the inlet mass flow rate, the inlet flow "
                 "area should be provided as well");
  }
  else if (!_area_pp)
    paramError("scalar_flux_pp",
               "If supplying the energy flow rate, the flow area should be provided as well");
}

ADReal
WCNSFVScalarFluxBC::computeQpResidual()
{
  const auto state = determineState();

  if (!isInflow())
    return varVelocity(state) * _normal * _passive_scalar(singleSidedFaceArg(), state);
  else if (_scalar_flux_pp)
    return -_scaling_factor * *_scalar_flux_pp / *_area_pp;

  return -_scaling_factor * inflowSpeed(state) * (*_scalar_value_pp);
}

bool
WCNSFVScalarFluxBC::isInflow() const
{
  if (_mdot_pp)
    return *_mdot_pp >= 0;
  else if (_velocity_pp)
    return *_velocity_pp >= 0;
  else if (_scalar_flux_pp)
    return *_scalar_flux_pp >= 0;

  mooseError(
      "Either mdot_pp or velocity_pp or scalar_flux_pp need to be provided OR this function "
      "must be overridden in derived classes if other input parameter combinations are valid. "
      "Neither mdot_pp nor velocity_pp are provided.");
  return true;
}
