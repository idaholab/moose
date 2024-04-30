//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVScalarTransportPhysics.h"

class WCNSFVFluidHeatTransferPhysics;

/**
 * Creates all the objects needed to solve the mixture terms for the weakly-compressible
 * and incompressible two-phase equations.
 * Can also add a phase transport equation
 */
class WCNSFVTwoPhaseMixturePhysics final : public WCNSFVScalarTransportPhysics
{
public:
  static InputParameters validParams();

  WCNSFVTwoPhaseMixturePhysics(const InputParameters & parameters);

protected:
private:
  void addFVKernels() override;
  void addMaterials() override;

  /**
   * Functions adding kernels for the other physics
   */
  void addPhaseInterfaceTerm();
  void addPhaseChangeEnergySource();
  void addPhaseDriftFluxTerm();

  /// Fluid heat transfer physics
  const WCNSFVFluidHeatTransferPhysics * _fluid_energy_physics;

  /// Convenience boolean to keep track of whether the phase transport equation is requested
  const bool _add_phase_equation;
  /// Convenience boolean to keep track of whether the fluid energy equation is present
  bool _has_energy_equation;

  /// Name of the liquid phase fraction (variable)
  const MooseFunctorName _liquid_phase_fraction;

  /// Name of the density of the other phase
  const MooseFunctorName _first_phase_density;
  /// Name of the dyanmic viscosity of the other phase
  const MooseFunctorName _first_phase_viscosity;
  /// Name of the specific heat of the other phase
  const MooseFunctorName _first_phase_specific_heat;
  /// Name of the thermal conductivity of the other phase
  const MooseFunctorName _first_phase_thermal_conductivity;

  /// Name of the other phase fraction
  const MooseFunctorName _other_phase_fraction_name;
  /// Name of the density of the other phase
  const MooseFunctorName _other_phase_density;
  /// Name of the dyanmic viscosity of the other phase
  const MooseFunctorName _other_phase_viscosity;
  /// Name of the specific heat of the other phase
  const MooseFunctorName _other_phase_specific_heat;
  /// Name of the thermal conductivity of the other phase
  const MooseFunctorName _other_phase_thermal_conductivity;

  /// Whether to define the mixture model internally or use fluid properties instead
  const bool _use_external_mixture_properties;

  /// Whether to add the drift flux momentum terms to the each component momentum equation
  const bool _use_drift_flux;
};
