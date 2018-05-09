//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWFLUIDSTATEFLASH_H
#define POROUSFLOWFLUIDSTATEFLASH_H

#include "GeneralUserObject.h"

class PorousFlowFluidStateFlash;

template <>
InputParameters validParams<PorousFlowFluidStateFlash>();

/// Phase state enum
enum class FluidStatePhaseEnum
{
  LIQUID,
  GAS,
  TWOPHASE
};

/**
 * Compositional flash routines for miscible multiphase flow classes
 */
class PorousFlowFluidStateFlash : public GeneralUserObject
{
public:
  PorousFlowFluidStateFlash(const InputParameters & parameters);

  void initialize() final{};
  void execute() final{};
  void finalize() final{};

  /**
   * Rachford-Rice equation for vapor fraction. Can be solved analytically for two
   * components in two phases, but must be solved iteratively using a root finding
   * algorithm for more components. This equation has the nice property
   * that it is monotonic in the interval [0,1], so that only a small number of
   * iterations are typically required to find the root.
   *
   * The Rachford-Rice equation can also be used to check whether the phase state
   * is two phase, single phase gas, or single phase liquid.
   * Evaluate f(v), the Rachford-Rice equation evaluated at the vapor mass fraction.
   *
   * If f(0) < 0, then the mixture is below the bubble point, and only a single phase
   * liquid can exist
   *
   * If f(1) > 0, then the mixture is above the dew point, and only a single phase gas exists.
   *
   * If f(0) >= 0 and f(1) <= 0, the mixture is between the bubble and dew points, and both
   * gas and liquid phases exist.
   *
   * @param vf vapor fraction
   * @param Zi mass fractions
   * @param Ki equilibrium constants
   * @return f(x)
   */
  Real rachfordRice(Real vf, std::vector<Real> & Zi, std::vector<Real> & Ki) const;

  /**
   * Derivative of Rachford-Rice equation wrt vapor fraction.
   * Has the nice property that it is strictly negative in the interval [0,1]
   *
   * @param vf vapor fraction
   * @param Zi mass fractions
   * @param Ki equilibrium constants
   * @return f'(x)
   */
  Real rachfordRiceDeriv(Real vf, std::vector<Real> & Zi, std::vector<Real> & Ki) const;

  /**
   * Solves Rachford-Rice equation to provide vapor mass fraction. For two components,
   * the analytical solution is used, while for cases with more than two components,
   * a Newton-Raphson iterative solution is calculated.
   *
   * @param Zi total mass fraction(s)
   * @param Ki equilibrium constant(s)
   * @return vapor mass fraction
   */
  Real vaporMassFraction(Real Z0, Real K0, Real K1) const;
  Real vaporMassFraction(std::vector<Real> & Zi, std::vector<Real> & Ki) const;

protected:
  /**
   * Determines the phase state gven the total mass fraction and equilibrium mass fractions
   *
   * @param Zi total mass fraction
   * @param Xi equilibrium mass fraction in liquid
   * @param Yi equilibrium mass fraction in gas
   * @param[out] phase_state the phase state (gas, liquid, two phase)
   */
  void phaseState(Real Zi, Real Xi, Real Yi, FluidStatePhaseEnum & phase_state) const;

  /// Maximum number of iterations for the Newton-Raphson routine
  const Real _nr_max_its;
  /// Tolerance for Newton-Raphson iterations
  const Real _nr_tol;
};

#endif // POROUSFLOWFLUIDSTATEFLASH_H
