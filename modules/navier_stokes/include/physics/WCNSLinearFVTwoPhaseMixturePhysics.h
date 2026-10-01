//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSLinearFVScalarTransportPhysics.h"

class WCNSLinearFVFluidHeatTransferPhysics;

/**
 * Creates all the objects needed to solve the mixture model for the weakly-compressible
 * and incompressible two-phase equations.
 * Can also add a phase transport equation
 */
class WCNSLinearFVTwoPhaseMixturePhysics final : public WCNSLinearFVScalarTransportPhysics
{
public:
  static InputParameters validParams();

  WCNSLinearFVTwoPhaseMixturePhysics(const InputParameters & parameters);

private:
  virtual void addFVKernels() override;

  virtual void addMaterials() override;
  virtual void checkIntegrity() const override;

  /**
   * Sets the velocity at which the phase fraction travels relative to the mixture. This is the
   * diffusion (drift) velocity, not the slip velocity: see
   * LinearWCNSFV2PSlipVelocityFunctorMaterial for the distinction. The base class calls this when
   * building the scalar advection kernel of the phase transport equation.
   */
  virtual void setSlipVelocityParams(InputParameters & params) const override;

  /// The phase transport equation is assembled as the dispersed phase mass equation, so every
  /// term carries the dispersed phase density.
  virtual MooseFunctorName scalarConservativeDensity(const VariableName & vname) const override;

  /// Sets the slip velocity, u_d - u_c, and the phase densities and fraction that weight it, on
  /// the kernels that consume the relative motion of the phases directly, namely the momentum and
  /// energy diffusion flux kernels
  void setRelativeVelocityParams(InputParameters & params) const;

  /**
   * Functions adding kernels for the other physics
   */
  void addPhaseInterfaceTerm();

  /// Adds d(rho_m)/dt to the pressure equation, the storage term of mixture continuity
  void addMassDensityTransientTerm();
  /// Builds d(rho_m)/dt once, on demand, and returns the functor name. Several equations need it.
  MooseFunctorName buildMixtureDensityTimeDerivative();
  /// Builds d(rho_m)/dp at fixed phase fraction, the coefficient the pressure driven part of
  /// the storage term carries onto the matrix diagonal
  MooseFunctorName buildMixtureDensityPressureDerivative();

  /// Adds the interfacial mass transfer source to the dispersed phase equation
  void addInterfacialMassTransferTerm();

  /// Creates the material that closes the interfacial mass transfer rate on the transported
  /// interfacial area concentration, for the case where it was not prescribed
  void addInterfacialMassTransferRateMaterial();

  /// Declares a ParsedFunctorMaterial from an expression over named functors, the pattern every
  /// derived quantity of this Physics is built with
  void addParsedFunctorMaterial(const std::string & object_name,
                                const std::string & property_name,
                                const std::string & expression,
                                const std::vector<std::string> & functor_names,
                                const std::vector<std::string> & functor_symbols,
                                bool output);

  /// Adds the latent heat absorbed or released by the interfacial mass transfer
  void addLatentHeatTransferTerm();
  void addPhaseChangeEnergySource();
  /**
   * Adds the pressure work the relative motion carries, (alpha - c_d) u_s . grad(p). The mixture
   * part of the term is added by the energy Physics; this is the part peculiar to the mixture.
   */
  void addEnergyPressureWorkDriftTerm();
  /// Adds the functor material holding the coefficient of the phase change energy term
  void addPhaseChangeCoefficientMaterial();
  void addPhaseDriftFluxTerm();
  /// Adds the enthalpy carried by the relative motion of the phases to the energy equation
  void addPhaseEnergyDriftFluxTerm();
  void addAdvectionSlipTerm();
  /// Adds the mass-weighted mixture specific heat, the weighting required for rho_m cp_m T to be
  /// the mixture enthalpy density
  void addMixtureSpecificHeatMaterial();

  /// Whether d(rho_m)/dt has already been constructed, so it is built at most once
  bool _built_drho_m_dt = false;
  bool _built_drho_m_dp = false;

  /// Fluid heat transfer physics
  const WCNSLinearFVFluidHeatTransferPhysics * _fluid_energy_physics;

  /// Convenience boolean to keep track of whether the phase transport equation is requested
  const bool _add_phase_equation;
  /// Convenience boolean to keep track of whether the fluid energy equation is present
  bool _has_energy_equation;

  /// Name of the first phase fraction (usually, liquid)
  const MooseFunctorName _phase_1_fraction_name;
  /// Name of the second phase fraction (usually, dispersed or advected by the liquid)
  const MooseFunctorName _phase_2_fraction_name;

  /// Name of the density of the first phase
  const MooseFunctorName _phase_1_density;
  /// Name of the dynamic viscosity of the first phase
  const MooseFunctorName _phase_1_viscosity;
  /// Name of the specific heat of the first phase
  const MooseFunctorName _phase_1_specific_heat;
  /// Name of the thermal conductivity of the first phase
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

  /// Whether the interfacial mass transfer rate is closed on the interfacial area rather than
  /// prescribed
  const bool _close_mass_transfer_on_area;

  /// Name of the interfacial mass transfer rate, the single rate every equation that needs it reads
  const MooseFunctorName _interfacial_mass_transfer;
};
