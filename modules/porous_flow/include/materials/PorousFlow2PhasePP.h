/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOW2PHASEPP_H
#define POROUSFLOW2PHASEPP_H

#include "PorousFlowVariableBase.h"

//Forward Declarations
class PorousFlow2PhasePP;

template<>
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
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /**
   * Assemble std::vectors of porepressure, saturation and temperature at the nodes
   * and quadpoints
   */
  void buildQpPPSS();

  /**
   * Effective saturation as a function of porepressure.
   * Default is constant saturation = 1.
   * Over-ride in derived classes to implement other effective saturation forulations
   *
   * @param pressure porepressure (Pa)
   * @return effective saturation
   */
  virtual Real effectiveSaturation(Real pressure) const;

  /**
   * Derivative of effective saturation wrt to porepressure.
   * Default = 0 for constant saturation.
   * Over-ride in derived classes to implement other effective saturation forulations
   *
   * @param pressure porepressure (Pa)
   * @return derivative of effective saturation wrt porepressure
   */
  virtual Real dEffectiveSaturation_dP(Real pressure) const;

  /**
   * Second derivative of effective saturation wrt to porepressure.
   * Default = 0 for constant saturation.
   * Over-ride in derived classes to implement other effective saturation forulations
   *
   * @param pressure porepressure (Pa)
   * @return second derivative of effective saturation wrt porepressure
   */
  virtual Real d2EffectiveSaturation_dP2(Real pressure) const;

  /// Nodal value of porepressure of the zero phase (eg, the water phase)
  const VariableValue & _phase0_porepressure_nodal;

  /// Quadpoint value of porepressure of the zero phase (eg, the water phase)
  const VariableValue & _phase0_porepressure_qp;

  /// Gradient(phase0_porepressure) at the qps
  const VariableGradient & _phase0_gradp_qp;

  /// Moose variable number of the phase0 porepressure
  const unsigned int _phase0_porepressure_varnum;

  /// PorousFlow variable number of the phase0 porepressure
  const unsigned int _p0var;

  /// Nodal value of porepressure of the one phase (eg, the gas phase)
  const VariableValue & _phase1_porepressure_nodal;

  /// Quadpoint value of porepressure of the one phase (eg, the gas phase)
  const VariableValue & _phase1_porepressure_qp;

  /// Gradient(phase1_porepressure) at the qps
  const VariableGradient & _phase1_gradp_qp;

  /// Moose variable number of the phase1 porepressure
  const unsigned int _phase1_porepressure_varnum;

  /// PorousFlow variable number of the phase1 porepressure
  const unsigned int _p1var;

  /// PorousFlow variable number of the temperature
  const unsigned int _tvar;
};

#endif //POROUSFLOW2PHASEPP_H
