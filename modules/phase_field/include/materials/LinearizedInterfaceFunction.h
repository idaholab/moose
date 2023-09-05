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
 * Creates the order parameter substitution used in linearized interface for phase field
 * models. \f$ \eta = \frac{1}{2}*(1 + tanh(\phi/\sqrt{2})) \f$.
 */
class LinearizedInterfaceFunction : public DerivativeParsedMaterialHelper, public ExpressionBuilder
{
public:
  static InputParameters validParams();

  LinearizedInterfaceFunction(const InputParameters & parameters);

protected:
  /// Coupled variable value for the nonlinear variable \f$ \phi \f$.
  EBTerm _phi;
};
