//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVHeatAdvectionPhysics.h"
#include "NSFVAction.h"

InputParameters
WCNSFVHeatAdvectionPhysics::validParams()
{
  InputParameters params = WCNSFVPhysicsBase::validParams();
  params.addClassDescription("Define the Navier Stokes weakly-compressible energy equation");

  /**
   * Equations used to set up the energy equation/enthalpy equation if it is required.
   */

  // params.addParam<FunctionName>(
  //     "initial_temperature", "300", "The initial temperature, assumed constant everywhere");

  // params.addParam<std::vector<std::vector<SubdomainName>>>(
  //     "thermal_conductivity_blocks",
  //     "The blocks where the user wants define different thermal conductivities.");

  // params.addParam<std::vector<MooseFunctorName>>(
  //     "thermal_conductivity",
  //     std::vector<MooseFunctorName>({NS::k}),
  //     "The name of the fluid thermal conductivity for each block");

  // params.addParam<MooseFunctorName>("specific_heat", NS::cp, "The name of the specific heat");

  // MultiMooseEnum en_inlet_types("fixed-temperature flux-mass flux-velocity heatflux");
  // params.addParam<MultiMooseEnum>("energy_inlet_types",
  //                                 en_inlet_types,
  //                                 "Types for the inlet boundaries for the energy equation.");

  // params.addParam<std::vector<std::string>>(
  //     "energy_inlet_function",
  //     std::vector<std::string>(),
  //     "Functions for fixed-value boundaries in the energy equation.");

  // MultiMooseEnum en_wall_types("fixed-temperature heatflux", "heatflux");
  // params.addParam<MultiMooseEnum>(
  //     "energy_wall_types", en_wall_types, "Types for the wall boundaries for the energy
  //     equation.");

  // params.addParam<std::vector<FunctionName>>(
  //     "energy_wall_function",
  //     std::vector<FunctionName>(),
  //     "Functions for Dirichlet/Neumann boundaries in the energy equation.");

  // params.addParam<std::vector<std::vector<SubdomainName>>>(
  //     "ambient_convection_blocks",
  //     std::vector<std::vector<SubdomainName>>(),
  //     "The blocks where the ambient convection is present.");

  // params.addParam<std::vector<MooseFunctorName>>(
  //     "ambient_convection_alpha",
  //     std::vector<MooseFunctorName>(),
  //     "The heat exchange coefficients for each block in 'ambient_convection_blocks'.");

  // params.addParam<std::vector<MooseFunctorName>>(
  //     "ambient_temperature",
  //     std::vector<MooseFunctorName>(),
  //     "The ambient temperature for each block in 'ambient_convection_blocks'.");

  // params.addParam<MooseFunctorName>(
  //     "external_heat_source",
  //     "The name of a functor which contains the external heat source for the energy equation.");
  // params.addParam<Real>(
  //     "external_heat_source_coeff", 1.0, "Multiplier for the coupled heat source term.");
  // params.addParam<bool>("use_external_enthalpy_material",
  //                       false,
  //                       "To indicate if the enthalpy material is set up outside of the action.");

  // params.addParam<MooseEnum>("energy_advection_interpolation",
  //                            adv_interpol_types,
  //                            "The numerical scheme to use for interpolating energy/temperature, "
  //                            "as an advected quantity, to the face.");

  // params.addParam<MooseEnum>("energy_face_interpolation",
  //                            face_interpol_types,
  //                            "The numerical scheme to interpolate the temperature/energy to the "
  //                            "face (separate from the advected quantity interpolation).");

  // params.addRangeCheckedParam<Real>(
  //   "energy_scaling", 1.0, "energy_scaling > 0.0", "The scaling factor for the energy
  //   variable.");

  return params;
}

WCNSFVHeatAdvectionPhysics::WCNSFVHeatAdvectionPhysics(const InputParameters & parameters)
  : WCNSFVPhysicsBase(parameters)
{
}
