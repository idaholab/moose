//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatConductionFV.h"
#include "NS.h"

/**
 * Creates all the objects needed to solve the porous media solid energy equation
 */
class PNSFVSolidHeatTransferPhysics final : public HeatConductionFV
{
public:
  static InputParameters validParams();

  PNSFVSolidHeatTransferPhysics(const InputParameters & parameters);

protected:
private:
  virtual void addSolverVariables() override;
  virtual void addFVKernels() override;
  virtual void addMaterials() override;

  // Note that we inherit:
  // - addInitialConditions from HeatConductionPhysicsBase
  // - addPreconditioning from HeatConductionPhysicsBase
  // - addFVBCs from HeatConductionFV

  virtual InputParameters getAdditionalRMParams() const override;

  /**
   * Functions adding kernels for the solid energy equation
   */
  void addPINSSolidEnergyTimeKernels();
  void addPINSSolidEnergyHeatConductionKernels();
  void addPINSSolidEnergyAmbientConvection();
  void addPINSSolidEnergyExternalHeatSource();

  /// Process thermal conductivity (multiple functor input options are available).
  /// Return true if we have vector thermal conductivity and false if scalar
  bool processThermalConductivity();

  /// Battery of additional checks on parameters
  void checkFluidAndSolidHeatTransferPhysicsParameters() const;

  /// Solid temperature name
  const NonlinearVariableName _solid_temperature_name;
  /// Fluid temperature name
  const NonlinearVariableName _fluid_temperature_name;
  /// Name of the porosity functor (usually material property)
  const MooseFunctorName _porosity_name;
  /// Name of the density functor (usually material property)
  const MooseFunctorName _density_name;
  /// Name of the specific heat functor (usually material property)
  const MooseFunctorName _specific_heat_name;
  /// Vector of subdomain groups where we want to have different thermal conduction
  std::vector<std::vector<SubdomainName>> _thermal_conductivity_blocks;
  /// Name of the thermal conductivity functor for each block-group
  std::vector<MooseFunctorName> _thermal_conductivity_name;

  /// Vector of subdomain groups where we want to have different ambient convection
  std::vector<std::vector<SubdomainName>> _ambient_convection_blocks;
  /// Name of the ambient convection heat transfer coefficients for each block-group
  std::vector<MooseFunctorName> _ambient_convection_alpha;
  /// Name of the solid domain temperature for each block-group
  std::vector<MooseFunctorName> _ambient_temperature;
};
