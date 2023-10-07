//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVPhysicsBase.h"

/**
 * Creates all the objects needed to solve the Navier Stokes energy equation
 */
class WCNSFVHeatAdvectionPhysics : public WCNSFVPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSFVHeatAdvectionPhysics(const InputParameters & parameters);

  /// GeneralUO not the right base class probably
  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};

protected:
private:
  void addNonlinearVariables() override;
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
  void addWCNSEnergyMixingLengthKernels();

  /// Functions adding boundary conditions for the incompressible simulation.
  /// These are used for weakly-compressible simulations as well.
  void addINSEnergyInletBC();
  void addINSEnergyWallBC();

  /// Add material to define an enthalpy functor material property for incompressible simulations
  void addEnthalpyMaterial();

  /// Process thermal conductivity (multiple functor input options are available).
  /// Return true if we have vector thermal conductivity and false if scalar
  bool processThermalConductivity();

  /// Name of the specific heat material property
  const MooseFunctorName _specific_heat_name;
  /// Subdomains where we want to have different thermal conduction
  const std::vector<std::vector<SubdomainName>> _thermal_conductivity_blocks;
  /// Name of the thermal conductivity functor for each block-group
  const std::vector<MooseFunctorName> _thermal_conductivity_name;
};
