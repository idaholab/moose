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
 * Base material designed to calculate fluid phase porepressure and saturation
 * for the single-phase situation assuming full saturation where porepressure
 * is the nonlinear variable.
 */
class PorousFlow1PhaseFullySaturated : public PorousFlowVariableBase
{
public:
  static InputParameters validParams();

  PorousFlow1PhaseFullySaturated(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Assemble std::vectors of porepressure, saturation and temperature at the quadpoints
   */
  void buildQpPPSS();

  /// Nodal or quadpoint value of porepressure of the fluid phase
  const VariableValue & _porepressure_var;
  /// Gradient(_porepressure at quadpoints)
  const VariableGradient & _gradp_qp_var;
  /// Moose variable number of the porepressure
  const unsigned int _porepressure_varnum;
  /// The PorousFlow variable number of the porepressure
  const unsigned int _p_var_num;
};
