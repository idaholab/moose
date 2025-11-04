//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarConstraint.h"

class GapConductanceStatefulConstraint : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  GapConductanceStatefulConstraint(const InputParameters & parameters);

protected:
  /**
   * Computes the residual for the LM equation, lambda = (k/l)*(T^(1) - PT^(2)).
   */
  virtual ADReal computeQpResidual(Moose::MortarType mortar_type) override;

  /// Thermal conductivity of the gap medium (e.g. air).
  const Real _k;

  /// Minimum gap distance allowed
  const Real _min_gap;

  ///@{ Displacement variables
  const std::vector<std::string> _disp_name;
  const unsigned int _n_disp;
  std::vector<const ADVariableValue *> _disp_secondary;
  std::vector<const ADVariableValue *> _disp_primary;
  ///@}

  /// Old stress variable (possibly nodally recovered property) on secondary surface
  const VariableValue & _stress_old;
  /// Old stress variable (possibly nodally recovered property) on primary surface
  const VariableValue & _stress_neighbor_old;
};
