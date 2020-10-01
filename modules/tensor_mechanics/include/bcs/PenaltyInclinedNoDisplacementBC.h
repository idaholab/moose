//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

class Function;

/**
 * Weakly enforce an inclined BC (u\dot n = 0) using a penalty method.
 */
class PenaltyInclinedNoDisplacementBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  PenaltyInclinedNoDisplacementBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  // An integer corresponding to the direction
  const unsigned int _component;

  /// Coupled displacement variables
  const unsigned int _ndisp;
  const std::vector<const VariableValue *> _disp;

  /// Variable IDs of coupled displacement variables
  const std::vector<unsigned int> _disp_var;

private:
  Real _penalty;
};
