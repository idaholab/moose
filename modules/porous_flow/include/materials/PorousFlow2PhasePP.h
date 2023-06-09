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
 * Material designed to calculate fluid phase porepressure and saturation
 * for the two-phase situation assuming phase porepressures as the nonlinear variables.
 */
template <bool is_ad>
class PorousFlow2PhasePPTempl : public PorousFlowVariableBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlow2PhasePPTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Assemble std::vectors of porepressure and saturation at the nodes
   * and quadpoints, and return the capillary pressure
   */
  GenericReal<is_ad> buildQpPPSS();

  /// Nodal or quadpoint value of porepressure of the zero phase (eg, the water phase)
  const GenericVariableValue<is_ad> & _phase0_porepressure;
  /// Gradient(phase0_porepressure) at the qps
  const GenericVariableGradient<is_ad> & _phase0_gradp_qp;
  /// Moose variable number of the phase0 porepressure
  const unsigned int _phase0_porepressure_varnum;
  /// PorousFlow variable number of the phase0 porepressure
  const unsigned int _p0var;
  /// Nodal or quadpoint value of porepressure of the one phase (eg, the gas phase)
  const GenericVariableValue<is_ad> & _phase1_porepressure;
  /// Gradient(phase1_porepressure) at the qps
  const GenericVariableGradient<is_ad> & _phase1_gradp_qp;
  /// Moose variable number of the phase1 porepressure
  const unsigned int _phase1_porepressure_varnum;
  /// PorousFlow variable number of the phase1 porepressure
  const unsigned int _p1var;
  /// Capillary pressure UserObject
  const PorousFlowCapillaryPressure & _pc_uo;

  usingPorousFlowVariableBaseMembers;
};

typedef PorousFlow2PhasePPTempl<false> PorousFlow2PhasePP;
typedef PorousFlow2PhasePPTempl<true> ADPorousFlow2PhasePP;
