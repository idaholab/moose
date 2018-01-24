//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWFLUIDSTATEBASE_H
#define POROUSFLOWFLUIDSTATEBASE_H

#include "PorousFlowFluidStateFlash.h"
#include "PorousFlowCapillaryPressure.h"

class PorousFlowFluidStateBase;

/// Data structure to pass calculated thermophysical properties
struct FluidStateProperties
{
  FluidStateProperties(){};
  FluidStateProperties(unsigned int n)
    : pressure(0.0),
      saturation(0.0),
      density(0.0),
      viscosity(1.0), // to guard against division by zero
      mass_fraction(n, 0.0),
      dsaturation_dp(0.0),
      dsaturation_dT(0.0),
      dsaturation_dz(0.0),
      ddensity_dp(0.0),
      ddensity_dT(0.0),
      ddensity_dz(0.0),
      dviscosity_dp(0.0),
      dviscosity_dT(0.0),
      dviscosity_dz(0.0),
      dmass_fraction_dp(n, 0.0),
      dmass_fraction_dT(n, 0.0),
      dmass_fraction_dz(n, 0.0){};

  Real pressure;
  Real saturation;
  Real density;
  Real viscosity;
  std::vector<Real> mass_fraction;
  Real dsaturation_dp;
  Real dsaturation_dT;
  Real dsaturation_dz;
  Real ddensity_dp;
  Real ddensity_dT;
  Real ddensity_dz;
  Real dviscosity_dp;
  Real dviscosity_dT;
  Real dviscosity_dz;
  std::vector<Real> dmass_fraction_dp;
  std::vector<Real> dmass_fraction_dT;
  std::vector<Real> dmass_fraction_dz;
};

template <>
InputParameters validParams<PorousFlowFluidStateBase>();

/**
 * Base class for fluid states for miscible multiphase flow in porous media.
 */
class PorousFlowFluidStateBase : public PorousFlowFluidStateFlash
{
public:
  PorousFlowFluidStateBase(const InputParameters & parameters);

  /**
   * The maximum number of phases in this model
   * @return number of phases
   */
  unsigned int numPhases() const;

  /**
   * The index of the aqueous phase
   * @return aqueous phase number
   */
  unsigned int aqueousPhaseIndex() const;

  /**
   * The index of the gas phase
   * @return gas phase number
   */
  unsigned int gasPhaseIndex() const;

  /**
   * The index of the aqueous fluid component
   * @return aqueous fluid component number
   */
  unsigned int aqueousComponentIndex() const;

  /**
   * The index of the gas fluid component
   * @return gas fluid component number
   */
  unsigned int gasComponentIndex() const;

  /**
   * Clears the contents of the FluidStateProperties data structure
   * @param[out] fsp FluidStateProperties data structure with all data initialized to 0
   */
  void clearFluidStateProperties(std::vector<FluidStateProperties> & fsp) const;

protected:
  /// Number of phases
  const unsigned int _num_phases;
  /// Number of components
  const unsigned int _num_components;
  /// Phase number of the aqueous phase
  const unsigned int _aqueous_phase_number;
  /// Phase number of the gas phase
  const unsigned int _gas_phase_number;
  /// Fluid component number of the aqueous component
  const unsigned int _aqueous_fluid_component;
  /// Fluid component number of the gas phase
  const unsigned int _gas_fluid_component;
  /// Universal gas constant (J/mol/K)
  const Real _R;
  /// Conversion from C to K
  const Real _T_c2k;
  /// Maximum number of iterations for the Newton-Raphson iterations
  const Real _nr_max_its;
  /// Tolerance for Newton-Raphson iterations
  const Real _nr_tol;
  /// Capillary pressure UserObject
  const PorousFlowCapillaryPressure & _pc_uo;
};

#endif // POROUSFLOWFLUIDSTATEBASE_H
