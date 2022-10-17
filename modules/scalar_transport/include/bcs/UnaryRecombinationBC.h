//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 * Models loss due to unary recombination, e.g. 2A -> B where A represents the variable this
 * boundary condition is applied to
 */
class UnaryRecombinationBC : public ADIntegratedBC
{
public:
  UnaryRecombinationBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADReal computeQpResidual() override;

  /// recombination rate coefficient
  const Real & _Kr;
};
