//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVEnergyFluxBC.h"
#include "INSFVEnergyVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVEnergyFluxBC);

InputParameters
WCNSFVEnergyFluxBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params += INSFVFlowBC::validParams();
  params.addClassDescription("Flux boundary conditions for energy advection.");

  params.addParam<Real>("scaling_factor", 1, "To scale the energy flux");

  // Three different ways to input an advected energy flux
  // 1) Postprocessor with the energy flow rate directly
  params.addParam<PostprocessorName>("energy_pp", "Postprocessor with the inlet energy flow rate");
  params.addParam<PostprocessorName>("area_pp", "Postprocessor with the inlet flow area");

  // 2) Postprocessors for velocity and energy, functors for specific heat and density
  params.addParam<PostprocessorName>("temperature_pp", "Postprocessor with the inlet temperature");
  params.addParam<MooseFunctorName>(NS::cp, "specific heat capacity functor");

  params.addParam<PostprocessorName>("velocity_pp", "Postprocessor with the inlet velocity norm");
  params.addParam<Point>("direction", Point(), "The direction of the flow at the boundary.");
  params.addParam<MooseFunctorName>(NS::density, "Density functor");

  // 3) Postprocessors for mass flow rate and energy, functor for specific heat
  params.addParam<PostprocessorName>("mdot_pp", "Postprocessor with the inlet mass flow rate");

  return params;
}

WCNSFVEnergyFluxBC::WCNSFVEnergyFluxBC(const InputParameters & params)
  : FVFluxBC(params),
    INSFVFlowBC(params),
    _scaling_factor(getParam<Real>("scaling_factor")),
    _temperature_pp(isParamValid("temperature_pp") ? &getPostprocessorValue("temperature_pp")
                                                   : nullptr),
    _energy_pp(isParamValid("energy_pp") ? &getPostprocessorValue("energy_pp") : nullptr),
    _velocity_pp(isParamValid("velocity_pp") ? &getPostprocessorValue("velocity_pp") : nullptr),
    _mdot_pp(isParamValid("mdot_pp") ? &getPostprocessorValue("mdot_pp") : nullptr),
    _area_pp(isParamValid("area_pp") ? &getPostprocessorValue("area_pp") : nullptr),
    _rho(isParamValid(NS::density) ? &getFunctor<ADReal>(NS::density) : nullptr),
    _cp(isParamValid(NS::cp) ? &getFunctor<ADReal>(NS::cp) : nullptr),
    _direction(getParam<Point>("direction")),
    _direction_specified_by_user(params.isParamSetByUser("direction"))
{
  if (_direction_specified_by_user)
  {
    if (_direction.is_zero())
      paramError("direction",
                 "The user should not define zero inlet/outlet advective flux values! Use noslip "
                 "boundary conditions for that purpose!");
    if (!MooseUtils::absoluteFuzzyEqual(_direction.norm(), 1.0, 1e-6))
      paramError("direction", "The direction should be a unit vector with a tolerance of 1e-6!");
  }

  if (!dynamic_cast<INSFVEnergyVariable *>(&_var))
    paramError("variable",
               "The variable argument to WCNSFVEnergyFluxBC must be of type INSFVEnergyVariable");

  // Density is often set as global parameters so it is not checked
  if (_energy_pp && (_velocity_pp || _direction_specified_by_user || _mdot_pp || _temperature_pp))
    mooseWarning("If setting the energy flow rate directly, "
                 "no need for inlet velocity (magnitude or direction), mass flow or temperature");

  // Need enough information if trying to use a mass flow rate postprocessor
  if (!_energy_pp)
  {
    if (!_temperature_pp)
      mooseError("If not providing the energy flow rate, "
                 "the inlet temperature should be provided");
    if (!_velocity_pp && !_mdot_pp)
      mooseError("If not providing the inlet energy flow rate, the inlet velocity or mass flow "
                 "should be provided");
    if (_velocity_pp && (!_rho || !_cp))
      mooseError("If providing the inlet velocity, the density, the area and the specific heat "
                 "capacity should be provided as well");
    if (_mdot_pp && (!_cp || !_area_pp))
      mooseError("If providing the inlet mass flow rate, the inlet specific heat capacity and flow"
                 " area should be provided as well");
  }
  else if (!_area_pp)
    paramError("energy_pp",
               "If supplying the energy flow rate, the flow area should be provided as well");
}

ADReal
WCNSFVEnergyFluxBC::computeQpResidual()
{
  if (_area_pp)
    if (MooseUtils::absoluteFuzzyEqual(*_area_pp, 0))
      mooseError("Surface area is 0");

  if (_energy_pp)
    return -_scaling_factor * *_energy_pp / *_area_pp;
  else
  {
    if (_face_info->neighborPtr() && !_direction_specified_by_user)
      mooseError("WCNSFVMomentumFluxBC can only be defined on an internal face if a direction "
                 "parameter is supplied!");

    if (_velocity_pp)
    {
      /*
       * We assume that positive and negative mass velocities correspond to fluid
       * entering / leaving the domain, respectively. There are two options for defining the
       * the vector for the inlet velocity direction.
       * 1. One can define it using the `direction` parameter. In this case the velocity and mdot
       * parameters are positive if the fluid flows along direction in a positive direction. If the
       * fluid is moving in the opposite direction they should be passed as negative values. The
       * `direction` parameter is needed when we define a boundary on an internal face.
       * 2. On boundaries, if `direction` is not defined, we use the surface normal and assume that
       * the fluid enters the domain with an opposite directionality.
       */
      const Point incoming_vector =
          !_direction_specified_by_user ? _face_info->normal() : _direction;
      const Real cos_angle = std::abs(incoming_vector * _face_info->normal());
      return -_scaling_factor * (*_rho)(singleSidedFaceArg()) * *_velocity_pp * cos_angle *
             (*_cp)(singleSidedFaceArg()) * *_temperature_pp;
    }
    else
      return -_scaling_factor * *_mdot_pp / *_area_pp * (*_cp)(singleSidedFaceArg()) *
             *_temperature_pp;
  }
}
