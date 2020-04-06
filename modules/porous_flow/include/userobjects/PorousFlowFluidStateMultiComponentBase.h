//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
   * @param Xnacl mass fraction of NaCl
   * @param Z total mass fraction of fluid component
   * @param qp quadpoint index
   * @param[out] fsp the FluidStateProperties struct containing all properties
   */
  virtual void thermophysicalProperties(Real pressure,
                                        Real temperature,
                                        Real Xnacl,
                                        Real Z,
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

  unsigned int getPressureIndex() const { return _pidx; };
  unsigned int getTemperatureIndex() const { return _Tidx; };
  unsigned int getZIndex() const { return _Zidx; };
  unsigned int getXIndex() const { return _Xidx; };

protected:
  /// Index of derivative wrt pressure
  const unsigned int _pidx;
  /// Index of derivative wrt total mass fraction Z
  const unsigned int _Zidx;
  /// Index of derivative wrt temperature
  const unsigned int _Tidx;
  /// Index of derivative wrt salt mass fraction X
  const unsigned int _Xidx;
};
