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
 * Material designed to calculate fluid phase porepressure and saturation
 * for the single-phase partially-saturation situation with hysteretic capillary pressure and
 * assuming porepressure is a nonlinear variable
 */
class PorousFlow1PhaseHysP : public PorousFlowHystereticCapillaryPressure
{
public:
  static InputParameters validParams();

  PorousFlow1PhaseHysP(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Computed nodal or quadpoint values of capillary pressure
  MaterialProperty<Real> & _pc;

  /**
   * Assemble std::vectors of porepressure and saturation
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
