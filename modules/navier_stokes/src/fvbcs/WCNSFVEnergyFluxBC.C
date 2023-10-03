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
  InputParameters params = WCNSFVFluxBCBase::validParams();
  params.addClassDescription("Flux boundary conditions for energy advection.");

  // Three different ways to input an advected energy flux
  // 1) Postprocessor with the energy flow rate directly
  // 2) Postprocessors for velocity and energy, functors for specific heat and density
  // 3) Postprocessors for mass flow rate and energy, functor for specific heat
  params.addParam<PostprocessorName>("energy_pp", "Postprocessor with the inlet energy flow rate");
  params.addParam<PostprocessorName>("temperature_pp", "Postprocessor with the inlet temperature");
  params.addRequiredParam<MooseFunctorName>(NS::cp, "specific heat capacity functor");
  params.addRequiredParam<MooseFunctorName>(NS::T_fluid, "temperature functor");
  return params;
}

WCNSFVEnergyFluxBC::WCNSFVEnergyFluxBC(const InputParameters & params)
  : WCNSFVFluxBCBase(params),
    _temperature_pp(isParamValid("temperature_pp") ? &getPostprocessorValue("temperature_pp")
                                                   : nullptr),
    _energy_pp(isParamValid("energy_pp") ? &getPostprocessorValue("energy_pp") : nullptr),
    _cp(getFunctor<ADReal>(NS::cp)),
    _temperature(getFunctor<ADReal>(NS::T_fluid))
{
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
    if (_mdot_pp && !_area_pp)
      mooseError("If providing the inlet mass flow rate, the flow"
                 " area should be provided as well");
  }
  else if (!_area_pp)
    paramError("energy_pp",
               "If supplying the energy flow rate, the flow area should be provided as well");
}

ADReal
WCNSFVEnergyFluxBC::computeQpResidual()
{
  const auto state = determineState();

  if (!isInflow())
  {
    const auto fa = singleSidedFaceArg();
    return varVelocity(state) * _normal * _rho(fa, state) * _cp(fa, state) *
           _temperature(fa, state);
  }
  else if (_energy_pp)
    return -_scaling_factor * *_energy_pp / *_area_pp;

  return -_scaling_factor * inflowMassFlux(state) * _cp(singleSidedFaceArg(), state) *
         (*_temperature_pp);
}

bool
WCNSFVEnergyFluxBC::isInflow() const
{
  if (_mdot_pp)
    return *_mdot_pp >= 0;
  else if (_velocity_pp)
    return *_velocity_pp >= 0;
  else if (_energy_pp)
    return *_energy_pp >= 0;

  mooseError(
      "Either mdot_pp or velocity_pp or energy_pp need to be provided OR this function must be "
      "overridden in derived classes if other input parameter combinations are valid. "
      "Neither mdot_pp nor velocity_pp are provided.");
  return true;
}
