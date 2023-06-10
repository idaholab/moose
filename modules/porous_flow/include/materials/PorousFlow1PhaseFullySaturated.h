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

/**
 * Material designed to calculate fluid phase porepressure and saturation
 * for the single-phase situation assuming full saturation where porepressure
 * is the nonlinear variable.
 */
template <bool is_ad>
class PorousFlow1PhaseFullySaturatedTempl : public PorousFlowVariableBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlow1PhaseFullySaturatedTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Assemble std::vectors of porepressure, saturation and temperature at the quadpoints
   */
  void buildQpPPSS();

  /// Nodal or quadpoint value of porepressure of the fluid phase
  const GenericVariableValue<is_ad> & _porepressure_var;
  /// Gradient(_porepressure at quadpoints)
  const GenericVariableGradient<is_ad> & _gradp_qp_var;
  /// Moose variable number of the porepressure
  const unsigned int _porepressure_varnum;
  /// The PorousFlow variable number of the porepressure
  const unsigned int _p_var_num;

  usingPorousFlowVariableBaseMembers;
};

typedef PorousFlow1PhaseFullySaturatedTempl<false> PorousFlow1PhaseFullySaturated;
typedef PorousFlow1PhaseFullySaturatedTempl<true> ADPorousFlow1PhaseFullySaturated;
