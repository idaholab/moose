//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addParam<MooseFunctorName>(NS::cp, "specific heat capacity functor");
  params.addRequiredParam<MooseFunctorName>(NS::T_fluid, "temperature functor");

  // Parameters to solve with the enthalpy variable
  params.addParam<MooseFunctorName>(NS::pressure, "pressure functor");
  params.addParam<MooseFunctorName>(NS::specific_enthalpy,
                                    "Fluid specific enthalpy functor. This can be specified to "
                                    "avoid using cp * T as the enthalpy");
  params.addParam<UserObjectName>(NS::fluid, "Fluid properties object");
  return params;
}

WCNSFVEnergyFluxBC::WCNSFVEnergyFluxBC(const InputParameters & params)
  : WCNSFVFluxBCBase(params),
    _temperature_pp(isParamValid("temperature_pp") ? &getPostprocessorValue("temperature_pp")
                                                   : nullptr),
    _energy_pp(isParamValid("energy_pp") ? &getPostprocessorValue("energy_pp") : nullptr),
    _cp(isParamValid(NS::cp) ? &getFunctor<ADReal>(NS::cp) : nullptr),
    _temperature(getFunctor<ADReal>(NS::T_fluid)),
    _pressure(isParamValid(NS::pressure) ? &getFunctor<ADReal>(NS::pressure) : nullptr),
    _h_fluid(isParamValid(NS::specific_enthalpy) ? &getFunctor<ADReal>(NS::specific_enthalpy)
                                                 : nullptr),
    _fluid(isParamValid(NS::fluid)
               ? &UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)
               : nullptr)
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

  // Currently an outlet
  if (!isInflow())
  {
    const auto fa = singleSidedFaceArg();
    return varVelocity(state) * _normal * _rho(fa, state) * enthalpy(fa, state, false);
  }
  // Currently an inlet with a set enthalpy
  else if (_energy_pp)
    return -_scaling_factor * *_energy_pp / *_area_pp;
  // Currently an inlet, compute the enthalpy
  return -_scaling_factor * inflowMassFlux(state) * enthalpy(singleSidedFaceArg(), state, true);
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

template <typename T>
ADReal
WCNSFVEnergyFluxBC::enthalpy(const T & loc_arg,
                             const Moose::StateArg & state,
                             const bool inflow) const
{
  // Outlet, use interior values
  if (!inflow)
  {
    if (_h_fluid)
      return (*_h_fluid)(loc_arg, state);
    else if (_fluid)
      return _fluid->h_from_p_T((*_pressure)(loc_arg, state), _temperature(loc_arg, state));
    else if (_temperature_pp)
      return (*_cp)(loc_arg, state) * _temperature(loc_arg, state);
  }
  else
  {
    if (_temperature_pp)
    {
      // Preferrable to use the fluid property if we know it
      if (_fluid)
        return _fluid->h_from_p_T((*_pressure)(loc_arg, state), (*_temperature_pp));
      else if (_cp)
        return (*_cp)(loc_arg, state) * (*_temperature_pp);
    }
  }
  mooseError("Should not reach here, constructor checks required functor inputs to flux BC");
}
