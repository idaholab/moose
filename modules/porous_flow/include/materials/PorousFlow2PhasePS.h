/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOW2PHASEPS_H
#define POROUSFLOW2PHASEPS_H

#include "PorousFlowVariableBase.h"

//Forward Declarations
class PorousFlow2PhasePS;

template<>
InputParameters validParams<PorousFlow2PhasePS>();

/**
 * Material designed to calculate fluid-phase porepressures at nodes
 */
class PorousFlow2PhasePS : public PorousFlowVariableBase
{
public:
  PorousFlow2PhasePS(const InputParameters & parameters);

protected:
  /**
   * Assemble std::vectors of porepressure, saturation and temperature at the nodes
   * and quadpoints
   */
  void buildQpPPSS();

  /**
   * Capillary pressure as a function of saturation.
   * Default is constant capillary pressure = 0.0.
   * Over-ride in derived classes to implement other capillary pressure forulations
   *
   * @param saturation saturation
   * @return capillary pressure
   */
  virtual Real capillaryPressure(Real saturation) const;

  /**
   * Derivative of capillary pressure wrt to saturation.
   * Default = 0 for constant capillary pressure.
   * Over-ride in derived classes to implement other capillary pressure forulations
   *
   * @param saturation saturation (Pa)
   * @return derivative of capillary pressure wrt saturation
   */
  virtual Real dCapillaryPressure_dS(Real pressure) const;

  /**
   * Second derivative of capillary pressure wrt to saturation.
   * Default = 0 for constant capillary pressure.
   * Over-ride in derived classes to implement other capillary pressure forulations
   *
   * @param saturation saturation (Pa)
   * @return second derivative of capillary pressure wrt saturation
   */
  virtual Real d2CapillaryPressure_dS2(Real pressure) const;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /// Nodal value of porepressure of the zero phase (eg, the gas phase)
  const VariableValue & _phase0_porepressure_nodal;

  /// Quadpoint value of porepressure of the zero phase (eg, the gas phase)
  const VariableValue & _phase0_porepressure_qp;

  /// Gradient(phase0_porepressure) at the qps
  const VariableGradient & _phase0_gradp_qp;

  /// Moose variable number of the phase0 porepressure
  const unsigned int _phase0_porepressure_varnum;

  /// PorousFlow variable number of the phase0 porepressure
  const unsigned int _pvar;

  /// Nodal value of saturation of the one phase (eg, the water phase)
  const VariableValue & _phase1_saturation_nodal;

  /// Quadpoint value of saturation of the one phase (eg, the water phase)
  const VariableValue & _phase1_saturation_qp;

  /// Gradient(phase1_saturation) at the qps
  const VariableGradient & _phase1_grads_qp;

  /// Moose variable number of the phase1 saturation
  const unsigned int _phase1_saturation_varnum;

  /// PorousFlow variable number of the phase1 saturation
  const unsigned int _svar;

  /// PorousFlow variable number of the temperature
  const unsigned int _tvar;

  /// Constant capillary pressure (Pa)
  const Real _pc;
};

#endif //POROUSFLOW2PHASEPS_H
