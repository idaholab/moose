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

private:
  virtual void addFVKernels() override;
  virtual void addMaterials() override;

  /// Adds the slip velocity parameters
  virtual void setSlipVelocityParams(InputParameters & params) const override;

  /**
   * Functions adding kernels for the other physics
   */
  void addPhaseInterfaceTerm();
  void addPhaseChangeEnergySource();
  void addPhaseDriftFluxTerm();
  void addAdvectionSlipTerm();

  /// Fluid heat transfer physics
  const WCNSFVFluidHeatTransferPhysics * _fluid_energy_physics;

  /// Convenience boolean to keep track of whether the phase transport equation is requested
  const bool _add_phase_equation;
  /// Convenience boolean to keep track of whether the fluid energy equation is present
  bool _has_energy_equation;

  /// Name of the first phase fraction (usually, liquid)
  const MooseFunctorName _phase_1_fraction_name;
  /// Name of the second phase fraction (usually, dispersed or advected by the liquid)
  const MooseFunctorName _phase_2_fraction_name;

  /// Name of the density of the other phase
  const MooseFunctorName _phase_1_density;
  /// Name of the dyanmic viscosity of the other phase
  const MooseFunctorName _phase_1_viscosity;
  /// Name of the specific heat of the other phase
  const MooseFunctorName _phase_1_specific_heat;
  /// Name of the thermal conductivity of the other phase
  const MooseFunctorName _phase_1_thermal_conductivity;

  /// Name of the density of the other phase
  const MooseFunctorName _phase_2_density;
  /// Name of the dynamic viscosity of the other phase
  const MooseFunctorName _phase_2_viscosity;
  /// Name of the specific heat of the other phase
  const MooseFunctorName _phase_2_specific_heat;
  /// Name of the thermal conductivity of the other phase
  const MooseFunctorName _phase_2_thermal_conductivity;

  /// Whether to define the mixture model internally or use fluid properties instead
  const bool _use_external_mixture_properties;

  /// Whether to add the drift flux momentum terms to each component momentum equation
  const bool _use_drift_flux;
  /// Whether to add the advection slip term to each component of the momentum equation
  const bool _use_advection_slip;
};
