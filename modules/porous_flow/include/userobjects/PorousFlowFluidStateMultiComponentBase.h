//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowFluidStateFlash.h"

/**
 * Compositional flash routines for miscible multiphase flow classes with multiple
 * fluid components
 */
class PorousFlowFluidStateMultiComponentBase : public PorousFlowFluidStateFlash
{
public:
  static InputParameters validParams();

  PorousFlowFluidStateMultiComponentBase(const InputParameters & parameters);

  /**
   * The index of the aqueous fluid component
   * @return aqueous fluid component number
   */
  unsigned int aqueousComponentIndex() const { return _aqueous_fluid_component; };

  /**
   * The index of the gas fluid component
   * @return gas fluid component number
   */
  unsigned int gasComponentIndex() const { return _gas_fluid_component; };

  /**
   * The index of the salt component
   * @return salt component number
   */
  unsigned int saltComponentIndex() const { return _salt_component; };

  /**
   * Determines the phase state gven the total mass fraction and equilibrium mass fractions
   *
   * @param Zi total mass fraction
   * @param Xi equilibrium mass fraction in liquid
   * @param Yi equilibrium mass fraction in gas
   * @param[out] phase_state the phase state (gas, liquid, two phase)
   */
  void phaseState(Real Zi, Real Xi, Real Yi, FluidStatePhaseEnum & phase_state) const;

  /**
   * Determines the complete thermophysical state of the system for a given set of
   * primary variables
   *
   * @param pressure gas phase pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param composition the numTotalMassFractions() independent total mass fractions z_0 ... z_{N-2}
   *          summed over all phases, followed by the NaCl mass fraction if hasSalt() is true. z_m
   *          is the total mass fraction of fluid component m + 1; the total mass fraction of
   *          component 0 (the aqueous component) is one minus their sum
   * @param qp quadpoint index
   * @param[out] fsp the FluidStateProperties struct containing all properties
   */
  virtual void thermophysicalProperties(Real pressure,
                                        Real temperature,
                                        const std::vector<Real> & composition,
                                        unsigned int qp,
                                        std::vector<FluidStateProperties> & fsp) const = 0;

  virtual void thermophysicalProperties(const ADReal & pressure,
                                        const ADReal & temperature,
                                        const std::vector<ADReal> & composition,
                                        unsigned int qp,
                                        std::vector<FluidStateProperties> & fsp) const = 0;

  /**
   * Total mass fraction of fluid component summed over all phases in the two-phase state
   * for a specified gas saturation
   *
   * @param pressure gas pressure (Pa)
   * @param temperature temperature (K)
   * @param Xnacl NaCl mass fraction (kg/kg)
   * @param saturation gas saturation (-)
   * @param qp quadpoint index
   * @return total mass fraction Z (-)
   */
  virtual Real totalMassFraction(
      Real pressure, Real temperature, Real Xnacl, Real saturation, unsigned int qp) const = 0;

  /// Whether the NaCl mass fraction is part of the composition passed to thermophysicalProperties()
  virtual bool hasSalt() const { return false; }

  /// Number of independent total mass fractions: one fewer than the number of fluid components,
  /// not counting salt
  unsigned int numTotalMassFractions() const { return _num_components - 1 - (hasSalt() ? 1 : 0); }

  ///@{ AD derivative indices of the primary variables, in the fixed layout
  /// pressure, temperature, {Z-block}, [Xnacl]. getXIndex() returns an invalid index if the fluid
  /// state has no salt
  unsigned int getPressureIndex() const { return 0; }
  unsigned int getTemperatureIndex() const { return 1; }
  unsigned int getZIndex(unsigned int m = 0) const { return 2 + m; }
  unsigned int getXIndex() const
  {
    return hasSalt() ? getZIndex(numTotalMassFractions()) : libMesh::invalid_uint;
  }
  ///@}

protected:
  /// Fluid component number of the aqueous component (always 0)
  const unsigned int _aqueous_fluid_component;
  /// Fluid component number of the gas phase (fixed by the derived class)
  unsigned int _gas_fluid_component;
  /// Salt component index (fixed by the derived class; invalid if the fluid state has no salt)
  unsigned int _salt_component;
  /// Whether to compute the internal energy of each phase
  const bool _compute_internal_energy;
  /// Whether to compute the enthalpy of each phase
  const bool _compute_enthalpy;
};
