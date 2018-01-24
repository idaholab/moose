//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOW2PHASEPS_H
#define POROUSFLOW2PHASEPS_H

#include "PorousFlowVariableBase.h"

// Forward Declarations
class PorousFlowCapillaryPressure;
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
   * and quadpoints
   */
  void buildQpPPSS();

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
   * @param saturation true saturation
   * @return capillary pressure (Pa)
   */
  virtual Real capillaryPressure(Real saturation) const;

  /**
   * Derivative of capillary pressure wrt to saturation.
   * Override in derived classes to implement other capillary pressure forulations
   *
   * @param saturation true saturation
   * @return derivative of capillary pressure wrt saturation
   */
  virtual Real dCapillaryPressure_dS(Real seff) const;

  /**
   * Second derivative of capillary pressure wrt to saturation.
   * Override in derived classes to implement other capillary pressure forulations
   *
   * @param saturation true saturation
   * @return second derivative of capillary pressure wrt saturation
   */
  virtual Real d2CapillaryPressure_dS2(Real saturation) const;

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
  /// Liquid residual saturation
  const Real _sat_lr;
  /// Derivative of effective saturation with respect to saturation
  const Real _dseff_ds;
  /// Capillary pressure UserObject
  /// Note: This pointer can be replaced with a reference once the deprecated PS
  /// materials have been removed
  const PorousFlowCapillaryPressure * _pc_uo;
};

#endif // POROUSFLOW2PHASEPS_H
