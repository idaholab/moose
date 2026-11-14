//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "NonADFunctorInterface.h"
#include "BlockRestrictable.h"
#include "MooseLinearVariableFV.h"

#include <array>

class SinglePhaseFluidProperties;
class ElemInfo;

/**
 * Relates the conserved mixture enthalpy density rho*h = sum_k alpha_k rho_k h_k(T) of a
 * two-phase diffuse-interface flow to its temperature. The phase densities are the constant ones
 * of the volume-fraction equation; the phase enthalpies come from fluid properties evaluated at a
 * fixed thermodynamic pressure. Provides the per-cell linearization of rho*h about the lagged
 * temperature that the energy kernels assemble with, recovers the temperature from the solved
 * rho*h, and sets the initial rho*h from the initial temperature and volume fraction.
 *
 * The energy unknown is rho*h rather than T so that the energy carried across the interface is
 * exactly the energy of the mass the volume-fraction transport moved: each phase's enthalpy is
 * advected with that phase's share of the mass flux the volume-fraction corrector published, and
 * the mixture heat capacity never appears as a coefficient that has to agree with the transport.
 * A temperature equation with a mixture rho cp in front of dT/dt, advected with the mixture mass
 * flux, does not have that property, and in an interface cell the mismatch between the mass the
 * cell gained and the heat capacity it was charged with shows up as a spurious temperature
 * change. With rho*h a uniform temperature stays uniform to round-off whatever the interface does.
 *
 * Because h_k(T) is nonlinear, the kernels work with the linearization of rho*h about the
 * temperature of the previous outer iteration in each cell,
 *   rho*h(T) = (rho*h)^* + C (T - T_lag) with C = d(rho*h)/dT = sum_k alpha_k rho_k cp_k,
 * so that T = rho*h / C + (T_lag - (rho*h)^* / C). Written in rho*h this way, advection and
 * conduction are implicit in the unknown and the matrix keeps the M-matrix structure of the
 * temperature equation, with its columns scaled by 1 / C.
 */
class ConservativeDiffuseInterfaceMixtureEnthalpy : public GeneralUserObject,
                                                    public NonADFunctorInterface,
                                                    public BlockRestrictable
{
public:
  static InputParameters validParams();

  ConservativeDiffuseInterfaceMixtureEnthalpy(const InputParameters & params);

  void initialSetup() override;
  void meshChanged() override;
  void initialize() override {}
  /// Sets rho*h from the initial temperature and volume fraction
  void execute() override;
  void finalize() override {}

  /// Indices of the two phases; the volume fraction is the liquid's
  enum Phase : unsigned int
  {
    LIQUID = 0,
    GAS = 1
  };
  static constexpr unsigned int num_phases = 2;

  /// Linearization of rho*h about the lagged temperature of a cell, at the current volume fraction
  struct CellLinearization
  {
    /// The lagged temperature
    Real temperature;
    /// rho*h of the current volume fraction at the lagged temperature
    Real enthalpy_density;
    /// d(rho*h)/dT = sum_k alpha_k rho_k cp_k at the lagged temperature
    Real heat_capacity;
    /// Mixture thermal conductivity 1 / sum_k (alpha_k / k_k), the phases in series, at the
    /// lagged temperature
    Real conductivity;
  };
  /// The linearization of rho*h in one cell
  CellLinearization cellLinearization(const ElemInfo & elem_info) const;

  /// The lagged temperature of a cell expressed through rho*h:
  /// T = rho*h / C + (T_lag - (rho*h)^* / C), returning the constant in parentheses
  static Real temperatureOffset(const CellLinearization & lin)
  {
    return lin.temperature - lin.enthalpy_density / lin.heat_capacity;
  }

  /// Specific enthalpy of a phase and its temperature derivative cp
  std::pair<Real, Real> phaseEnthalpy(Phase phase, Real temperature) const;

  /// rho*h of a volume fraction at a temperature
  Real enthalpyDensity(Real alpha, Real temperature, const Moose::FaceArg & face) const;

  /// The constant density of a phase on a face
  Real phaseDensity(Phase phase, const Moose::FaceArg & face) const;

  /// Overwrites the temperature with the root of sum_k alpha_k rho_k h_k(T) = rho*h in every cell
  void recoverTemperature();

private:
  /// Looks up the enthalpy and temperature variables, which must be linear FV variables
  void cacheVariables();

  /// The conserved rho*h variable
  const VariableName _enthalpy_name;
  /// The temperature variable recovered from rho*h
  const VariableName _temperature_name;
  /// The liquid volume fraction
  const Moose::Functor<Real> & _alpha;
  /// The constant phase densities, indexed by Phase
  const std::array<const Moose::Functor<Real> *, num_phases> _density;
  /// The fluid properties giving h_k(p, T), cp_k and k_k of each phase, indexed by Phase
  std::array<const SinglePhaseFluidProperties *, num_phases> _fp;
  /// Pressure at which the phase enthalpies are evaluated. The flow solves a reduced pressure,
  /// which is not a thermodynamic pressure, and the enthalpy of a low-Mach flow does not depend on
  /// its pressure variations.
  const Real _thermodynamic_pressure;

  MooseLinearVariableFVReal * _enthalpy_var = nullptr;
  MooseLinearVariableFVReal * _temperature_var = nullptr;
};
