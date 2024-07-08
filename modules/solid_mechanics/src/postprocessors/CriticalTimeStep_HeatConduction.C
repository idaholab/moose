//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CriticalTimeStep_HeatConduction.h"
#include "MooseTypes.h"

registerMooseObject("SolidMechanicsApp", CriticalTimeStep_HeatConduction);

InputParameters
CriticalTimeStep_HeatConduction::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();
  params.addClassDescription("Computes and reports the critical time step for the explicit solver "
                             "(for transient heat conduction).");
  params.addParam<MaterialPropertyName>(
      "density",
      "density",
      "Name of Material Property or a constant real number defining the density of the material.");
  params.addParam<MaterialPropertyName>("thermal_conductivity",
                                        "thermal_conductivity",
                                        "Name of Material Property or a constant real number "
                                        "defining the thermal conductivity of the material.");
  params.addParam<MaterialPropertyName>("specific_heat",
                                        "specific_heat",
                                        "Name of Material Property or a constant real number "
                                        "defining the specific heat capacity of the material.");
  params.addParam<Real>("dimension_factor",
                        6.,
                        "Denominator in the critical time step calculation for heat conduction"
                        "The value should be 2,4,6 for spatial dimension 1D,2D,3D respectively. "
                        "Defaults to the most conservative (3D) case.");
  params.addParam<Real>("factor", 1.0, "Factor to multiply to the critical time step.");
  return params;
}

CriticalTimeStep_HeatConduction::CriticalTimeStep_HeatConduction(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    GuaranteeConsumer(this),
    _material_density(getMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("density"))),
    _density_scaling(parameters.isParamSetByUser("density_scaling")
                         ? &getMaterialPropertyByName<Real>("density_scaling")
                         : nullptr),
    _thermal_conductivity(
        getMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("thermal_conductivity"))),
    _specific_heat(
        getMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("specific_heat"))),
    _dimension_factor(getParam<Real>("dimension_factor")),
    _factor(getParam<Real>("factor")),
    _critical_time(parameters.isParamValid("critical_time"))
{
}

void
CriticalTimeStep_HeatConduction::initialSetup()
{
}

void
CriticalTimeStep_HeatConduction::initialize()
{
  _critical_time = std::numeric_limits<Real>::max();
}

void
CriticalTimeStep_HeatConduction::execute()
{
  const Real dens = _material_density[0];

  const Real k = _thermal_conductivity[0];
  const Real c_p = _specific_heat[0];

  // Thermal diffusivity
  if (dens <= 0)
    mooseError(
        "Non-positive effective density found in CriticalTimeStep_HeatConduction postprocessor.");
  if (c_p <= 0)
    mooseError("Non-positive specific heat capacity found in CriticalTimeStep_HeatConduction "
               "postprocessor.");

  Real thm_diff = k / dens / c_p;
  if (thm_diff <= 0)
    mooseError(
        "Non-positive thermal diffusivity found in CriticalTimeStep_HeatConduction postprocessor.");

  // Critical Time Step
  _critical_time = std::min(_factor * _current_elem->hmin() * _current_elem->hmin() /
                                (_dimension_factor * thm_diff),
                            _critical_time);
}

void
CriticalTimeStep_HeatConduction::finalize()
{
  gatherMin(_critical_time);
}

Real
CriticalTimeStep_HeatConduction::getValue() const
{
  return _critical_time;
}

void
CriticalTimeStep_HeatConduction::threadJoin(const UserObject & y)
{
  const auto & pps = static_cast<const CriticalTimeStep_HeatConduction &>(y);
  _critical_time = std::min(pps._critical_time, _critical_time);
}
