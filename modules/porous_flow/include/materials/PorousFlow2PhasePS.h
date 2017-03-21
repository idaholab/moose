/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOW2PHASEPS_H
#define POROUSFLOW2PHASEPS_H

#include "PorousFlowVariableBase.h"

// Forward Declarations
class PorousFlow2PhasePS;

template <>
InputParameters validParams<PorousFlow2PhasePS>();

/**
 * Material designed to calculate fluid-phase porepressures and saturations at nodes
 * and qps using a specified capillary pressure formulation
 */
class PorousFlow2PhasePS : public PorousFlowVariableBase
{
public:
  PorousFlow2PhasePS(const InputParameters & parameters);

protected:
  /**
   * Assemble std::vectors of porepressure and saturation at the nodes
   * and quadpoints, and return the effective saturation
   */
  Real buildQpPPSS();

  /**
   * Effective saturation of liquid phase
   * @param saturation true saturation
   * @return effective saturation
   */
  virtual Real effectiveSaturation(Real saturation) const;

  /**
   * Capillary pressure as a function of saturation.
   * Default is constant capillary pressure = 0.0.
   * Override in derived classes to implement other capillary pressure forulations
   *
   * @param seff effective saturation
   * @return capillary pressure (Pa)
   */
  virtual Real capillaryPressure(Real seff) const;

  /**
   * Derivative of capillary pressure wrt to saturation.
   * Default = 0 for constant capillary pressure.
   * Override in derived classes to implement other capillary pressure forulations
   *
   * @param seff effective saturation
   * @return derivative of capillary pressure wrt effective saturation
   */
  virtual Real dCapillaryPressure_dS(Real seff) const;

  /**
   * Second derivative of capillary pressure wrt to saturation.
   * Default = 0 for constant capillary pressure.
   * Override in derived classes to implement other capillary pressure forulations
   *
   * @param seff effective saturation (Pa)
   * @return second derivative of capillary pressure wrt effective saturation
   */
  virtual Real d2CapillaryPressure_dS2(Real seff) const;

  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Nodal or quadpoint value of porepressure of the zero phase (eg, the gas phase)
  const VariableValue & _phase0_porepressure;

  /// Gradient(phase0_porepressure) at the qps
  const VariableGradient & _phase0_gradp_qp;

  /// Moose variable number of the phase0 porepressure
  const unsigned int _phase0_porepressure_varnum;

  /// PorousFlow variable number of the phase0 porepressure
  const unsigned int _pvar;

  /// Nodal or quadpoint value of saturation of the one phase (eg, the water phase)
  const VariableValue & _phase1_saturation;

  /// Gradient(phase1_saturation) at the qps
  const VariableGradient & _phase1_grads_qp;

  /// Moose variable number of the phase1 saturation
  const unsigned int _phase1_saturation_varnum;

  /// PorousFlow variable number of the phase1 saturation
  const unsigned int _svar;

  /// Constant capillary pressure (Pa)
  const Real _pc;

  /// Liquid residual saturation
  const Real _sat_lr;

  /// Derivative of effective saturation with respect to saturation
  const Real _dseff_ds;
};

#endif // POROUSFLOW2PHASEPS_H
