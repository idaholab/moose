//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDependencies.h"

PorousFlowDependencies::PorousFlowDependencies()
{
  // Action dependencies
  _deps.insertDependency("PorousFlowActionBase", "PorousFlowDarcyVelocityComponent");

  _deps.insertDependency("PorousFlowSinglePhaseBase", "PorousFlowActionBase");
  _deps.insertDependency("PorousFlowSinglePhaseBase", "PorousFlowEffectiveStressCoupling");
  _deps.insertDependency("PorousFlowSinglePhaseBase", "PorousFlowHeatConduction");
  _deps.insertDependency("PorousFlowSinglePhaseBase", "PorousFlowEnergyTimeDerivative");
  _deps.insertDependency("PorousFlowSinglePhaseBase", "PorousFlowHeatVolumetricExpansion");

  _deps.insertDependency("PorousFlowFullySaturated", "PorousFlowSinglePhaseBase");
  _deps.insertDependency("PorousFlowFullySaturated", "PorousFlowFullySaturatedDarcyFlow");
  _deps.insertDependency("PorousFlowFullySaturated", "PorousFlowFullySaturatedAdvectiveFlux");
  _deps.insertDependency("PorousFlowFullySaturated", "PorousFlowMassTimeDerivative");
  _deps.insertDependency("PorousFlowFullySaturated", "PorousFlowMassVolumetricExpansion");
  _deps.insertDependency("PorousFlowFullySaturated", "PorousFlowFullySaturatedHeatAdvection");
  _deps.insertDependency("PorousFlowFullySaturated", "PorousFlowFullySaturatedUpwindHeatAdvection");

  _deps.insertDependency("PorousFlowBasicTHM", "PorousFlowSinglePhaseBase");
  _deps.insertDependency("PorousFlowBasicTHM", "PorousFlowFullySaturatedDarcyBase");
  _deps.insertDependency("PorousFlowBasicTHM", "PorousFlowFullySaturatedMassTimeDerivative");
  _deps.insertDependency("PorousFlowBasicTHM", "PorousFlowFullySaturatedHeatAdvection");

  _deps.insertDependency("PorousFlowUnsaturated", "PorousFlowSinglePhaseBase");
  _deps.insertDependency("PorousFlowUnsaturated", "PorousFlowAdvectiveFlux");
  _deps.insertDependency("PorousFlowUnsaturated", "PorousFlowMassTimeDerivative");
  _deps.insertDependency("PorousFlowUnsaturated", "PorousFlowMassVolumetricExpansion");
  _deps.insertDependency("PorousFlowUnsaturated", "PorousFlowHeatAdvection");

  // AuxKernel dependencies
  _deps.insertDependency("PorousFlowDarcyVelocityComponent", "relative_permeability_qp");
  _deps.insertDependency("PorousFlowDarcyVelocityComponent", "density_qp");
  _deps.insertDependency("PorousFlowDarcyVelocityComponent", "viscosity_qp");
  _deps.insertDependency("PorousFlowDarcyVelocityComponent", "permeability_qp");
  _deps.insertDependency("PorousFlowDarcyVelocityComponent", "pressure_saturation_qp");

  _deps.insertDependency("PorousFlowDarcyVelocityComponentLowerDimensional",
                         "PorousFlowDarcyVelocityComponent");

  _deps.insertDependency("PorousFlowPropertyAux", "pressure_saturation_qp");
  _deps.insertDependency("PorousFlowPropertyAux", "temperature_qp");
  _deps.insertDependency("PorousFlowPropertyAux", "fluid_properties_qp");
  _deps.insertDependency("PorousFlowPropertyAux", "mass_fraction_qp");
  _deps.insertDependency("PorousFlowPropertyAux", "relative_permeability_qp");
  _deps.insertDependency("PorousFlowPropertyAux", "chemistry_qp");
  _deps.insertDependency("PorousFlowPropertyAux", "mineral_qp");
  _deps.insertDependency("PorousFlowPropertyAux", "porosity_qp");

  // BC dependencies
  _deps.insertDependency("PorousFlowHalfCubicSink", "PorousFlowSink");
  _deps.insertDependency("PorousFlowHalfGaussianSink", "PorousFlowSink");
  _deps.insertDependency("PorousFlowPiecewiseLinearSink", "PorousFlowSink");

  _deps.insertDependency("PorousFlowSink", "pressure_saturation_nodal");
  _deps.insertDependency("PorousFlowSink", "temperature_nodal");
  _deps.insertDependency("PorousFlowSink", "mass_fraction_nodal");
  _deps.insertDependency("PorousFlowSink", "fluid_properties_nodal");
  _deps.insertDependency("PorousFlowSink", "relative_permeability_nodal");
  _deps.insertDependency("PorousFlowSink", "enthalpy_nodal");
  _deps.insertDependency("PorousFlowSink", "internal_energy_nodal");
  _deps.insertDependency("PorousFlowSink", "permeability_qp");
  _deps.insertDependency("PorousFlowSink", "thermal_conductivity_qp");

  _deps.insertDependency("PorousFlowOutflowBC", "pressure_saturation_qp");
  _deps.insertDependency("PorousFlowOutflowBC", "density_qp");
  _deps.insertDependency("PorousFlowOutflowBC", "permeability_qp");
  _deps.insertDependency("PorousFlowOutflowBC", "viscosity_nodal");
  _deps.insertDependency("PorousFlowOutflowBC", "density_nodal");
  _deps.insertDependency("PorousFlowOutflowBC", "relative_permeability_nodal");
  _deps.insertDependency("PorousFlowOutflowBC", "mass_fraction_nodal");
  _deps.insertDependency("PorousFlowOutflowBC", "enthalpy_nodal");
  _deps.insertDependency("PorousFlowOutflowBC", "thermal_conductivity_qp");
  _deps.insertDependency("PorousFlowOutflowBC", "temperature_qp");

  // Dirac kernel dependencies
  _deps.insertDependency("PorousFlowPeacemanBorehole", "PorousFlowLineSink");
  _deps.insertDependency("PorousFlowPolyLineSink", "PorousFlowLineSink");

  _deps.insertDependency("PorousFlowLineSink", "pressure_saturation_qp");
  _deps.insertDependency("PorousFlowLineSink", "temperature_qp");
  _deps.insertDependency("PorousFlowLineSink", "thermal_conductivity_qp");
  _deps.insertDependency("PorousFlowLineSink", "relative_permeability_nodal");
  _deps.insertDependency("PorousFlowLineSink", "density_nodal");
  _deps.insertDependency("PorousFlowLineSink", "viscosity_nodal");
  _deps.insertDependency("PorousFlowLineSink", "enthalpy_nodal");
  _deps.insertDependency("PorousFlowLineSink", "internal_energy_nodal");
  _deps.insertDependency("PorousFlowLineSink", "relative_permeability_nodal");
  _deps.insertDependency("PorousFlowLineSink", "mass_fraction_nodal");

  // Kernel dependencies
  _deps.insertDependency("PorousFlowAdvectiveFlux", "PorousFlowDarcyBase");
  _deps.insertDependency("PorousFlowAdvectiveFlux", "mass_fraction_nodal");
  _deps.insertDependency("PorousFlowAdvectiveFlux", "relative_permeability_nodal");

  _deps.insertDependency("PorousFlowFullySaturatedAdvectiveFlux", "PorousFlowDarcyBase");
  _deps.insertDependency("PorousFlowFullySaturatedAdvectiveFlux", "mass_fraction_nodal");

  _deps.insertDependency("PorousFlowBasicAdvection", "darcy_velocity_qp");

  _deps.insertDependency("PorousFlowDarcyBase", "permeability_qp");
  _deps.insertDependency("PorousFlowDarcyBase", "density_qp");
  _deps.insertDependency("PorousFlowDarcyBase", "density_nodal");
  _deps.insertDependency("PorousFlowDarcyBase", "viscosity_nodal");
  _deps.insertDependency("PorousFlowDarcyBase", "pressure_saturation_qp");

  _deps.insertDependency("PorousFlowDesorpedMassTimeDerivative", "porosity_qp");

  _deps.insertDependency("PorousFlowDesorpedMassVolumetricExpansion", "porosity_qp");
  _deps.insertDependency("PorousFlowDesorpedMassVolumetricExpansion", "volumetric_strain_qp");

  _deps.insertDependency("PorousFlowDispersiveFlux", "density_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "viscosity_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "mass_fraction_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "porosity_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "diffusivity_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "relative_permeability_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "permeability_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "pressure_saturation_qp");

  _deps.insertDependency("PorousFlowEffectiveStressCoupling", "effective_pressure_qp");

  _deps.insertDependency("PorousFlowEnergyTimeDerivative", "porosity_nodal");
  _deps.insertDependency("PorousFlowEnergyTimeDerivative", "nearest_qp_nodal");
  _deps.insertDependency("PorousFlowEnergyTimeDerivative", "matrix_internal_energy_nodal");
  _deps.insertDependency("PorousFlowEnergyTimeDerivative", "density_nodal");
  _deps.insertDependency("PorousFlowEnergyTimeDerivative", "enthalpy_nodal");
  _deps.insertDependency("PorousFlowEnergyTimeDerivative", "internal_energy_nodal");
  _deps.insertDependency("PorousFlowEnergyTimeDerivative", "pressure_saturation_nodal");

  _deps.insertDependency("PorousFlowFullySaturatedDarcyBase", "permeability_qp");
  _deps.insertDependency("PorousFlowFullySaturatedDarcyBase", "density_qp");
  _deps.insertDependency("PorousFlowFullySaturatedDarcyBase", "viscosity_qp");
  _deps.insertDependency("PorousFlowFullySaturatedDarcyBase", "pressure_saturation_qp");

  _deps.insertDependency("PorousFlowFullySaturatedDarcyFlow", "PorousFlowFullySaturatedDarcyBase");
  _deps.insertDependency("PorousFlowFullySaturatedDarcyFlow", "mass_fraction_qp");

  _deps.insertDependency("PorousFlowFullySaturatedHeatAdvection",
                         "PorousFlowFullySaturatedDarcyBase");
  _deps.insertDependency("PorousFlowFullySaturatedHeatAdvection", "enthalpy_qp");

  _deps.insertDependency("PorousFlowFullySaturatedUpwindHeatAdvection", "PorousFlowDarcyBase");
  _deps.insertDependency("PorousFlowFullySaturatedUpwindHeatAdvection", "enthalpy_nodal");

  _deps.insertDependency("PorousFlowFullySaturatedMassTimeDerivative", "biot_modulus_qp");
  _deps.insertDependency("PorousFlowFullySaturatedMassTimeDerivative", "thermal_expansion_qp");
  _deps.insertDependency("PorousFlowFullySaturatedMassTimeDerivative", "density_qp");
  _deps.insertDependency("PorousFlowFullySaturatedMassTimeDerivative", "pressure_saturation_qp");
  _deps.insertDependency("PorousFlowFullySaturatedMassTimeDerivative", "temperature_qp");
  _deps.insertDependency("PorousFlowFullySaturatedMassTimeDerivative", "volumetric_strain_qp");

  _deps.insertDependency("PorousFlowHeatAdvection", "PorousFlowDarcyBase");
  _deps.insertDependency("PorousFlowHeatAdvection", "enthalpy_nodal");
  _deps.insertDependency("PorousFlowHeatAdvection", "relative_permeability_nodal");

  _deps.insertDependency("PorousFlowHeatConduction", "thermal_conductivity_qp");
  _deps.insertDependency("PorousFlowHeatConduction", "temperature_qp");

  _deps.insertDependency("PorousFlowHeatVolumetricExpansion", "porosity_nodal");
  _deps.insertDependency("PorousFlowHeatVolumetricExpansion", "nearest_qp_nodal");
  _deps.insertDependency("PorousFlowHeatVolumetricExpansion", "matrix_internal_energy_nodal");
  _deps.insertDependency("PorousFlowHeatVolumetricExpansion", "density_nodal");
  _deps.insertDependency("PorousFlowHeatVolumetricExpansion", "internal_energy_nodal");
  _deps.insertDependency("PorousFlowHeatVolumetricExpansion", "pressure_saturation_nodal");
  _deps.insertDependency("PorousFlowHeatVolumetricExpansion", "volumetric_strain_qp");

  _deps.insertDependency("PorousFlowMassRadioactiveDecay", "porosity_nodal");
  _deps.insertDependency("PorousFlowMassRadioactiveDecay", "nearest_qp_nodal");
  _deps.insertDependency("PorousFlowMassRadioactiveDecay", "density_nodal");
  _deps.insertDependency("PorousFlowMassRadioactiveDecay", "pressure_saturation_nodal");
  _deps.insertDependency("PorousFlowMassRadioactiveDecay", "mass_fraction_nodal");

  _deps.insertDependency("PorousFlowMassTimeDerivative", "porosity_nodal");
  _deps.insertDependency("PorousFlowMassTimeDerivative", "nearest_qp_nodal");
  _deps.insertDependency("PorousFlowMassTimeDerivative", "density_nodal");
  _deps.insertDependency("PorousFlowMassTimeDerivative", "pressure_saturation_nodal");
  _deps.insertDependency("PorousFlowMassTimeDerivative", "mass_fraction_nodal");

  _deps.insertDependency("PorousFlowMassVolumetricExpansion", "porosity_nodal");
  _deps.insertDependency("PorousFlowMassVolumetricExpansion", "nearest_qp_nodal");
  _deps.insertDependency("PorousFlowMassVolumetricExpansion", "density_nodal");
  _deps.insertDependency("PorousFlowMassVolumetricExpansion", "pressure_saturation_nodal");
  _deps.insertDependency("PorousFlowMassVolumetricExpansion", "mass_fraction_nodal");
  _deps.insertDependency("PorousFlowMassVolumetricExpansion", "volumetric_strain_qp");

  _deps.insertDependency("PorousFlowPlasticHeatEnergy", "PlasticHeatEnergy");
  _deps.insertDependency("PorousFlowPlasticHeatEnergy", "nearest_qp_nodal");
  _deps.insertDependency("PorousFlowPlasticHeatEnergy", "porosity_nodal");

  _deps.insertDependency("PorousFlowPreDis", "pressure_saturation_nodal");
  _deps.insertDependency("PorousFlowPreDis", "porosity_nodal");
  _deps.insertDependency("PorousFlowPreDis", "chemistry_nodal");
  _deps.insertDependency("PorousFlowPreDis", "mineral_nodal");

  // Material dependencies
  _deps.insertDependency("density_qp", "fluid_properties_qp");
  _deps.insertDependency("density_nodal", "fluid_properties_nodal");
  _deps.insertDependency("viscosity_qp", "fluid_properties_qp");
  _deps.insertDependency("viscosity_nodal", "fluid_properties_nodal");
  _deps.insertDependency("internal_energy_qp", "fluid_properties_qp");
  _deps.insertDependency("internal_energy_nodal", "fluid_properties_nodal");
  _deps.insertDependency("enthalpy_qp", "fluid_properties_qp");
  _deps.insertDependency("enthalpy_nodal", "fluid_properties_nodal");

  _deps.insertDependency("darcy_velocity_qp", "permeability_qp");
  _deps.insertDependency("darcy_velocity_qp", "fluid_properties_qp");
  _deps.insertDependency("darcy_velocity_qp", "relative_permeability_qp");
  _deps.insertDependency("darcy_velocity_qp", "pressure_saturation_qp");

  _deps.insertDependency("chemistry_nodal", "pressure_saturation_nodal");
  _deps.insertDependency("chemistry_nodal", "porosity_nodal");
  _deps.insertDependency("chemistry_nodal", "temperature_nodal");
  _deps.insertDependency("chemistry_qp", "pressure_saturation_qp");
  _deps.insertDependency("chemistry_qp", "porosity_qp");
  _deps.insertDependency("chemistry_qp", "temperature_qp");

  _deps.insertDependency("mineral_nodal", "pressure_saturation_nodal");
  _deps.insertDependency("mineral_nodal", "porosity_nodal");
  _deps.insertDependency("mineral_nodal", "chemistry_nodal");
  _deps.insertDependency("mineral_qp", "pressure_saturation_qp");
  _deps.insertDependency("mineral_qp", "porosity_qp");
  _deps.insertDependency("mineral_qp", "chemistry_qp");

  _deps.insertDependency("biot_modulus_nodal", "porosity_nodal");
  _deps.insertDependency("biot_modulus_qp", "porosity_qp");

  _deps.insertDependency("thermal_expansion_nodal", "porosity_nodal");
  _deps.insertDependency("thermal_expansion_qp", "porosity_qp");

  _deps.insertDependency("fluid_properties_nodal", "pressure_saturation_nodal");
  _deps.insertDependency("fluid_properties_nodal", "temperature_nodal");
  _deps.insertDependency("fluid_properties_nodal", "fluid_state_nodal");
  _deps.insertDependency("fluid_properties_qp", "pressure_saturation_qp");
  _deps.insertDependency("fluid_properties_qp", "temperature_qp");
  _deps.insertDependency("fluid_properties_qp", "fluid_state_qp");

  _deps.insertDependency("mass_fraction_nodal", "fluid_state_nodal");
  _deps.insertDependency("mass_fraction_qp", "fluid_state_qp");

  _deps.insertDependency("fluid_state_nodal", "temperature_nodal");
  _deps.insertDependency("fluid_state_qp", "temperature_qp");

  _deps.insertDependency("diffusivity_qp", "porosity_qp");
  _deps.insertDependency("diffusivity_qp", "pressure_saturation_qp");

  _deps.insertDependency("effective_pressure_nodal", "pressure_saturation_nodal");
  _deps.insertDependency("effective_pressure_qp", "pressure_saturation_qp");

  _deps.insertDependency("matrix_internal_energy_nodal", "temperature_nodal");

  _deps.insertDependency("permeability_qp", "porosity_qp");

  _deps.insertDependency("relative_permeability_nodal", "pressure_saturation_nodal");
  _deps.insertDependency("relative_permeability_qp", "pressure_saturation_qp");

  _deps.insertDependency("thermal_conductivity_qp", "porosity_qp");
  _deps.insertDependency("thermal_conductivity_qp", "pressure_saturation_qp");

  _deps.insertDependency("gravitational_density_qp", "porosity_qp");
  _deps.insertDependency("gravitational_density_qp", "fluid_properties_qp");

  _deps.insertDependency("porosity_nodal", "effective_pressure_nodal");
  _deps.insertDependency("porosity_nodal", "pressure_saturation_nodal");
  _deps.insertDependency("porosity_nodal", "temperature_nodal");
  _deps.insertDependency("porosity_qp", "effective_pressure_qp");
  _deps.insertDependency("porosity_qp", "pressure_saturation_qp");
  _deps.insertDependency("porosity_qp", "temperature_qp");
  //_deps.insertDependency("porosity_qp", "volumetric_strain_qp");

  // following is so that anything derived from PorousFlowVariableBase (with pf_material_type =
  // pressure_saturation) will add PorousFlowHysteresisOrder at the nodes or qps, if a
  // PorousflowHysteresisOrder appears in the input file
  _deps.insertDependency("pressure_saturation_nodal", "hysteresis_order_nodal");
  _deps.insertDependency("pressure_saturation_qp", "hysteresis_order_qp");

  // Postprocessor dependencies
  _deps.insertDependency("PorousFlowFluidMass", "porosity_nodal");
  _deps.insertDependency("PorousFlowFluidMass", "density_nodal");
  _deps.insertDependency("PorousFlowFluidMass", "mass_fraction_nodal");
  _deps.insertDependency("PorousFlowFluidMass", "pressure_saturation_nodal");
  _deps.insertDependency("PorousFlowHeatEnergy", "porosity_nodal");
  _deps.insertDependency("PorousFlowHeatEnergy", "matrix_internal_energy_nodal");
  _deps.insertDependency("PorousFlowHeatEnergy", "density_nodal");
  _deps.insertDependency("PorousFlowHeatEnergy", "internal_energy_nodal");
  _deps.insertDependency("PorousFlowHeatEnergy", "pressure_saturation_nodal");

  // UserObject dependencies
  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorBase", "permeability_qp");
  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorBase", "pressure_saturation_qp");
  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorBase", "density_qp");

  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorSaturated",
                         "PorousFlowAdvectiveFluxCalculatorBase");
  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorSaturated", "density_nodal");
  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorSaturated", "viscosity_nodal");

  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent",
                         "PorousFlowAdvectiveFluxCalculatorSaturated");
  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent",
                         "mass_fraction_nodal");

  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorUnsaturated",
                         "PorousFlowAdvectiveFluxCalculatorSaturated");
  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorUnsaturated",
                         "relative_permeability_nodal");

  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent",
                         "PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent");
  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent",
                         "relative_permeability_nodal");

  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorSaturatedHeat",
                         "PorousFlowAdvectiveFluxCalculatorSaturated");
  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorSaturatedHeat", "enthalpy_nodal");

  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat",
                         "PorousFlowAdvectiveFluxCalculatorSaturatedHeat");
  _deps.insertDependency("PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat",
                         "relative_permeability_nodal");
}
