//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOW2PHASEPP_H
#define POROUSFLOW2PHASEPP_H

#include "PorousFlowVariableBase.h"

// Forward Declarations
class PorousFlowCapillaryPressure;
class PorousFlow2PhasePP;

template <>
InputParameters validParams<PorousFlow2PhasePP>();

/**
 * Base material designed to calculate fluid phase porepressure and saturation
 * for the two-phase situation assuming phase porepressures as the nonlinear variables.
 * Inherit and over-ride effectiveSaturation, dEffectiveSaturation, and
 * d2EffectiveSaturation to implement specific capillary pressure functions
 */
class PorousFlow2PhasePP : public PorousFlowVariableBase
{
public:
  PorousFlow2PhasePP(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Assemble std::vectors of porepressure and saturation at the nodes
   * and quadpoints, and return the capillary pressure
   */
  Real buildQpPPSS();

  /**
   * Effective saturation as a function of porepressure (a negative quantity).
   * Default is constant saturation = 1.
   * Over-ride in derived classes to implement other effective saturation forulations
   *
   * @param pressure porepressure (Pa): a negative quantity
   * @return effective saturation
   */
  virtual Real effectiveSaturation(Real pressure) const;

  /**
   * Derivative of effective saturation wrt to p.
   * Default = 0 for constant saturation.
   * Over-ride in derived classes to implement other effective saturation forulations
   *
   * @param pressure porepressure (Pa): a negative quantity
   * @return derivative of effective saturation wrt porepressure
   */
  virtual Real dEffectiveSaturation_dP(Real pressure) const;

  /**
   * Second derivative of effective saturation wrt to porepressure.
   * Default = 0 for constant saturation.
   * Over-ride in derived classes to implement other effective saturation forulations
   *
   * @param pressure porepressure (Pa): a negative quantity
   * @return second derivative of effective saturation wrt porepressure
   */
  virtual Real d2EffectiveSaturation_dP2(Real pressure) const;

  /// Nodal or quadpoint value of porepressure of the zero phase (eg, the water phase)
  const VariableValue & _phase0_porepressure;
  /// Gradient(phase0_porepressure) at the qps
  const VariableGradient & _phase0_gradp_qp;
  /// Moose variable number of the phase0 porepressure
  const unsigned int _phase0_porepressure_varnum;
  /// PorousFlow variable number of the phase0 porepressure
  const unsigned int _p0var;
  /// Nodal or quadpoint value of porepressure of the one phase (eg, the gas phase)
  const VariableValue & _phase1_porepressure;
  /// Gradient(phase1_porepressure) at the qps
  const VariableGradient & _phase1_gradp_qp;
  /// Moose variable number of the phase1 porepressure
  const unsigned int _phase1_porepressure_varnum;
  /// PorousFlow variable number of the phase1 porepressure
  const unsigned int _p1var;
  /// Capillary pressure UserObject
  /// Note: This pointer can be replaced with a reference once the deprecated PP
  /// materials have been removed
  const PorousFlowCapillaryPressure * _pc_uo;
};

#endif // POROUSFLOW2PHASEPP_H
