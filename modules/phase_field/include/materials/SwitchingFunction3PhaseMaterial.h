//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeParsedMaterialHelper.h"
#include "ExpressionBuilder.h"

// Forward Declarations

/**
 * Material class to provide switching functions that prevent formation of a
 * third phase at a two-phase interface. See Folch and Plapp, Phys. Rev. E, v. 72,
 * 011602 (2005).
 */
class SwitchingFunction3PhaseMaterial : public DerivativeParsedMaterialHelper,
                                        public ExpressionBuilder
{
public:
  static InputParameters validParams();

  SwitchingFunction3PhaseMaterial(const InputParameters & parameters);

protected:
  /// Coupled variable values for order parameters.
  EBTerm _eta_i;
  EBTerm _eta_j;
  EBTerm _eta_k;
};
