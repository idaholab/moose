//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarConstraint.h"

class GapFluxModelBase;

/**
 * This Constraint implements thermal contact using a "gap
 * conductance" model in which the flux is represented by an
 * independent "Lagrange multiplier" like variable.
 */
class ModularGapConductanceConstraint : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  ModularGapConductanceConstraint(const InputParameters & parameters);

protected:
  /**
   * Computes the residual for the LM equation, lambda = (k/l)*(T^(1) - PT^(2)).
   */
  virtual ADReal computeQpResidual(Moose::MortarType mortar_type) override;

  /// Gap flux model names
  std::vector<UserObjectName> _gap_flux_model_names;

  /// Gap flux models
  std::vector<const GapFluxModelBase *> _gap_flux_models;

  ///@{ Displacement variables
  const std::vector<std::string> _disp_name;
  const unsigned int _n_disp;
  std::vector<const ADVariableValue *> _disp_secondary;
  std::vector<const ADVariableValue *> _disp_primary;
  ///@}
};
