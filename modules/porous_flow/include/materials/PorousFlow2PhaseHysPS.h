//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowHystereticCapillaryPressure.h"

/**
 * Material designed to calculate the 2 porepressures and 2 saturations, as well as derivatives of
 * them, as well as capillary pressure, in two-phase situations with hysteretic capillary pressure,
 * assuming the liquid porepressure and gas saturation are the nonlinear variables.
 */
class PorousFlow2PhaseHysPS : public PorousFlowHystereticCapillaryPressure
{
public:
  static InputParameters validParams();

  PorousFlow2PhaseHysPS(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Computed nodal or quadpoint values of capillary pressure
  MaterialProperty<Real> & _pc;

  /**
   * Assemble std::vectors of porepressure and saturation at the nodes
   * and quadpoints
   */
  void buildQpPPSS();

  /// Nodal or quadpoint value of porepressure of phase zero (the liquid phase)
  const VariableValue & _phase0_porepressure;
  /// Gradient(phase0_porepressure) at the qps
  const VariableGradient & _phase0_gradp_qp;
  /// Moose variable number of the phase0 porepressure
  const unsigned int _phase0_porepressure_varnum;
  /// PorousFlow variable number of the phase0 porepressure
  const unsigned int _pvar;
  /// Nodal or quadpoint value of saturation of phase one (the gas phase)
  const VariableValue & _phase1_saturation;
  /// Gradient(phase1_saturation) at the qps
  const VariableGradient & _phase1_grads_qp;
  /// Moose variable number of the phase1 saturation
  const unsigned int _phase1_saturation_varnum;
  /// PorousFlow variable number of the phase1 saturation
  const unsigned int _svar;
};
