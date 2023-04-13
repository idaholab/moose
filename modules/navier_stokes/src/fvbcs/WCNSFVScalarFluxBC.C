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
  return params;
}

WCNSFVScalarFluxBC::WCNSFVScalarFluxBC(const InputParameters & params)
  : WCNSFVFluxBCBase(params),
    _scalar_value_pp(isParamValid("scalar_value_pp") ? &getPostprocessorValue("scalar_value_pp")
                                                     : nullptr),
    _scalar_flux_pp(isParamValid("scalar_flux_pp") ? &getPostprocessorValue("scalar_flux_pp")
                                                   : nullptr)
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
  else
  {
    /*
     * We assume the following orientation: The supplied mass flow and velocity magnitude need to
     * be positive if:
     * 1. No direction parameter is supplied and we want to define an inlet condition (similarly,
     * if the mass flow/velocity magnitude are negative we define an outlet)
     * 2. If the fluid flows aligns with the direction parameter specified by the user.
     * (similarly, if the postprocessor values are negative we assume the fluid flows backwards
     * with respect to the direction parameter)
     */
    checkForInternalDirection();

    const Point incoming_vector = !_direction_specified_by_user ? _face_info->normal() : _direction;
    const Real cos_angle = std::abs(incoming_vector * _face_info->normal());
    if (_velocity_pp)
    {
      return -_scaling_factor * *_velocity_pp * *_scalar_value_pp * cos_angle;
    }
    else
      return -_scaling_factor * *_mdot_pp / *_area_pp /
             (*_rho)(singleSidedFaceArg(), determineState()) / cos_angle * *_scalar_value_pp;
  }
}
