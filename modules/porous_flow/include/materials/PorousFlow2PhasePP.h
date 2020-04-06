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
 * Base material designed to calculate fluid phase porepressure and saturation
 * for the two-phase situation assuming phase porepressures as the nonlinear variables.
 */
class PorousFlow2PhasePP : public PorousFlowVariableBase
{
public:
  static InputParameters validParams();

  PorousFlow2PhasePP(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Assemble std::vectors of porepressure and saturation at the nodes
   * and quadpoints, and return the capillary pressure
   */
  Real buildQpPPSS();

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
  const PorousFlowCapillaryPressure & _pc_uo;
};
