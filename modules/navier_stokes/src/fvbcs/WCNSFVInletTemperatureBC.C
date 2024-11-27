//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVInletTemperatureBC.h"
#include "INSFVEnergyVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVInletTemperatureBC);

InputParameters
WCNSFVInletTemperatureBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params += INSFVFlowBC::validParams();

  params.addParam<Real>("scaling_factor", 1, "To scale the velocity");

  // Three different ways to input temperature
  // 1) Postprocessor with the temperature value directly
  params.addParam<PostprocessorName>("temperature_pp", "Postprocessor with the inlet temperature");

  // 2) Postprocessors for velocity and energy, functors for specific heat and density
  params.addParam<PostprocessorName>("energy_pp", "Postprocessor with the inlet energy flow rate");
  params.addParam<PostprocessorName>("velocity_pp", "Postprocessor with the inlet velocity norm");
  params.addParam<MooseFunctorName>(NS::density, "Density functor");
  params.addParam<MooseFunctorName>(NS::cp, "specific heat capacity functor");

  // 3) Postprocessors for mass flow rate and energy, functor for specific heat
  params.addParam<PostprocessorName>("mdot_pp", "Postprocessor with the inlet mass flow rate");

  return params;
}

WCNSFVInletTemperatureBC::WCNSFVInletTemperatureBC(const InputParameters & params)
  : FVDirichletBCBase(params),
    INSFVFlowBC(params),
    _scaling_factor(getParam<Real>("scaling_factor")),
    _temperature_pp(isParamValid("temperature_pp") ? &getPostprocessorValue("temperature_pp")
                                                   : nullptr),
    _energy_pp(isParamValid("energy_pp") ? &getPostprocessorValue("energy_pp") : nullptr),
    _velocity_pp(isParamValid("velocity_pp") ? &getPostprocessorValue("velocity_pp") : nullptr),
    _mdot_pp(isParamValid("mdot_pp") ? &getPostprocessorValue("mdot_pp") : nullptr),
    _area_pp(isParamValid("area_pp") ? &getPostprocessorValue("area_pp") : nullptr),
    _rho(isParamValid(NS::density) ? &getFunctor<ADReal>(NS::density) : nullptr),
    _cp(isParamValid(NS::cp) ? &getFunctor<ADReal>(NS::cp) : nullptr)
{
  if (!dynamic_cast<INSFVEnergyVariable *>(&_var))
    paramError(
        "variable",
        "The variable argument to WCNSFVInletTemperatureBC must be of type INSFVEnergyVariable");

  // Density is often set as global parameters so it is not checked
  if (_temperature_pp && (_velocity_pp || _mdot_pp || _energy_pp))
    mooseWarning(
        "If setting the temperature directly, no need for inlet velocity, mass flow or energy");

  // Need enough information if trying to use a mass flow rate postprocessor
  if (!_temperature_pp)
  {
    if (!_energy_pp)
      mooseError("If not providing the temperature, the energy flow rate should be provided");
    if (!_velocity_pp && !_mdot_pp)
      mooseError("If not providing the inlet temperature, the inlet velocity or mass flow should "
                 "be provided");
    if (_velocity_pp && (!_rho || !_cp || !_area_pp))
      mooseError("If providing the inlet velocity, the density, the area and the specific heat "
                 "capacity should be provided as well");
    if (_mdot_pp && !_cp)
      mooseError("If providing the inlet mass flow rate, the inlet specific heat capacity should "
                 "be provided as well");
  }
  else if (params.isParamSetByUser("scaling_factor"))
    paramError("scaling_factor",
               "The scaling factor is meant to adjust for a different area or "
               "mass flow rate, it should not be set if the temperature is set directly");
}

ADReal
WCNSFVInletTemperatureBC::boundaryValue(const FaceInfo & fi, const Moose::StateArg & state) const
{

  if (_area_pp)
    if (MooseUtils::absoluteFuzzyEqual(*_area_pp, 0))
      mooseError("Surface area is 0");

  if (_temperature_pp)
    return *_temperature_pp;
  else if (_velocity_pp)
  {
    ADReal rho = (*_rho)(singleSidedFaceArg(&fi), state);
    ADReal cp = (*_cp)(singleSidedFaceArg(&fi), state);

    return _scaling_factor * (*_energy_pp) / (*_area_pp * rho * *_velocity_pp * cp);
  }
  else
  {
    ADReal cp = (*_cp)(singleSidedFaceArg(&fi), state);

    return _scaling_factor * (*_energy_pp) / (*_mdot_pp * cp);
  }
}
