//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowVariableBase.h"

class PorousFlowCapillaryPressure;

/**
 * Material designed to calculate fluid-phase porepressures and saturations at nodes
 * and qps using a specified capillary pressure formulation
 */
class PorousFlow2PhasePS : public PorousFlowVariableBase
{
public:
  static InputParameters validParams();

  PorousFlow2PhasePS(const InputParameters & parameters);

protected:
  /**
   * Assemble std::vectors of porepressure and saturation at the nodes
   * and quadpoints
   */
  void buildQpPPSS();

  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Nodal or quadpoint value of porepressure of phase zero (eg, the liquid phase)
  const VariableValue & _phase0_porepressure;
  /// Gradient(phase0_porepressure) at the qps
  const VariableGradient & _phase0_gradp_qp;
  /// Moose variable number of the phase0 porepressure
  const unsigned int _phase0_porepressure_varnum;
  /// PorousFlow variable number of the phase0 porepressure
  const unsigned int _pvar;
  /// Nodal or quadpoint value of saturation of phase one (eg, the gas phase)
  const VariableValue & _phase1_saturation;
  /// Gradient(phase1_saturation) at the qps
  const VariableGradient & _phase1_grads_qp;
  /// Moose variable number of the phase1 saturation
  const unsigned int _phase1_saturation_varnum;
  /// PorousFlow variable number of the phase1 saturation
  const unsigned int _svar;
  /// Capillary pressure UserObject
  const PorousFlowCapillaryPressure & _pc_uo;
};
