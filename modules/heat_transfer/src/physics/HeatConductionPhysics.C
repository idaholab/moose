//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionPhysics.h"
#include "ADHeatConduction.h"
#include "HeatConductionFE.h"

registerMooseObject("HeatConductionApp", HeatConductionPhysics);

InputParameters
HeatConductionPhysics::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params.addClassDescription("Add the heat conduction physics");

  params.transferParam<MaterialPropertyName>(ADHeatConduction::validParams(),
                                             "thermal_conductivity");
  params.addParam<VariableName>("temperature_name", "T", "Variable name for the temperature");
  params.addParam<VariableName>("heat_source_var", "Variable providing the heat source");

  // Boundary conditions
  params.addParam<std::vector<BoundaryName>>("heat_flux_boundaries",
                                             "Boundaries on which to apply a heat flux");
  params.addParam<std::vector<BoundaryName>>("insulated_boundaries",
                                             "Boundaries on which to apply a zero heat flux");
  params.addParam<std::vector<BoundaryName>>("fixed_temperature_boundaries",
                                             "Boundaries on which to apply a fixed templates");
  params.addParam<std::vector<MooseFunctorName>>(
      "boundary_heat_fluxes", "Functors to compute the heat flux on each 'heat_flux' boundary'");
  params.addParam<std::vector<MooseFunctorName>>(
      "boundary_temperatures",
      "Functors to compute the heat flux on each 'fixed_temperature' boundary'");
  params.addParamNamesToGroup(
      "heat_flux_boundaries insulated_boundaries fixed_temperature_boundaries boundary_heat_fluxes "
      "boundary_temperatures",
      "Thermal boundaries");

  return params;
}

HeatConductionPhysics::HeatConductionPhysics(const InputParameters & parameters)
  : PhysicsBase(parameters), _temperature_name(getParam<VariableName>("temperature_name"))
{
  // Parameter checking
  checkVectorParamsSameLength<BoundaryName, MooseFunctorName>("heat_flux_boundaries",
                                                              "boundary_heat_fluxes");
  checkVectorParamsSameLength<BoundaryName, MooseFunctorName>("fixed_temperature_boundaries",
                                                              "boundary_temperatures");
  checkVectorParamsNoOverlap<BoundaryName>(
      {"heat_flux_boundaries", "insulated_boundaries", "fixed_temperature_boundaries"});
}

void
HeatConductionPhysics::createDiscretizedPhysics()
{
  std::cout << getDiscretization().type() << std::endl;
  std::cout << getDiscretization().name() << std::endl;
  if (getDiscretization().type() == "ContinuousGalerkin")
    _discretized_physics = std::make_unique<HeatConductionFE>(parameters());
}
