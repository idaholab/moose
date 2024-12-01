//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NavierStokesPhysicsBase.h"
#include "WCNSFVCoupledAdvectionPhysicsHelper.h"
#include "NS.h"

/**
 * Creates all the objects needed to solve the Navier Stokes energy equation
 */
class WCNSFVFluidHeatTransferPhysics final : public NavierStokesPhysicsBase,
                                             public WCNSFVCoupledAdvectionPhysicsHelper
{
public:
  static InputParameters validParams();

  WCNSFVFluidHeatTransferPhysics(const InputParameters & parameters);

  /// Get the name of the fluid temperature variable
  const NonlinearVariableName & getFluidTemperatureName() const { return _fluid_temperature_name; }

  /// Get the name of the specific heat material property
  const MooseFunctorName & getSpecificHeatName() const { return _specific_heat_name; }
  MooseFunctorName getSpecificEnthalpyName() const { return NS::specific_enthalpy; }
  const std::vector<MooseFunctorName> & getThermalConductivityName() const
  {
    return _thermal_conductivity_name;
  }

  /// Get the ambient convection parameters for parameter checking
  const std::vector<std::vector<SubdomainName>> & getAmbientConvectionBlocks() const
  {
    return _ambient_convection_blocks;
  }
  /// Name of the ambient convection heat transfer coefficients for each block-group
  const std::vector<MooseFunctorName> & getAmbientConvectionHTCs() const
  {
    return _ambient_convection_alpha;
  }

  /// Whether the physics is actually creating the heat equation
  bool hasEnergyEquation() const { return _has_energy_equation; }

protected:
private:
  void actOnAdditionalTasks() override;
  void addSolverVariables() override;
  void addInitialConditions() override;
  void addFVKernels() override;
  void addFVBCs() override;
  void addMaterials() override;

  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  /**
   * Functions adding kernels for the incompressible / weakly compressible energy equation
   * If the material properties are not constant, some of these can be used for
   * weakly-compressible simulations as well.
   */
  void addINSEnergyTimeKernels();
  void addWCNSEnergyTimeKernels();
  void addINSEnergyHeatConductionKernels();
  void addINSEnergyAdvectionKernels();
  void addINSEnergyAmbientConvection();
  void addINSEnergyExternalHeatSource();

  /// Functions adding boundary conditions for the incompressible simulation.
  /// These are used for weakly-compressible simulations as well.
  void addINSEnergyInletBC();
  void addINSEnergyWallBC();

  /// Process thermal conductivity (multiple functor input options are available).
  /// Return true if we have vector thermal conductivity and false if scalar
  bool processThermalConductivity();

  /// A boolean to help compatibility with the old Modules/NavierStokesFV syntax
  const bool _has_energy_equation;
  /// Fluid temperature name
  NonlinearVariableName _fluid_temperature_name;
  /// Name of the specific heat material property
  MooseFunctorName _specific_heat_name;
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

  /// Energy inlet boundary types
  MultiMooseEnum _energy_inlet_types;
  /// Functors describing the inlet boundary values. See energy_inlet_types for what the functors actually represent
  std::vector<MooseFunctorName> _energy_inlet_functors;
  /// Energy wall boundary types
  MultiMooseEnum _energy_wall_types;
  /// Functors describing the wall boundary values. See energy_wall_types for what the functors actually represent
  std::vector<MooseFunctorName> _energy_wall_functors;
};
