//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeFunctionMaterialBase.h"

// Forward Declarations

/**
 * Material class that creates the math free energy and its derivatives
 * for use with CHParsed and SplitCHParsed. \f$ F = \frac14(1 + c)^2(1 - c)^2 \f$.
 */
class MathFreeEnergy : public DerivativeFunctionMaterialBase
{
public:
  static InputParameters validParams();

  MathFreeEnergy(const InputParameters & parameters);

protected:
  virtual Real computeF();
  virtual Real computeDF(unsigned int j_var);
  virtual Real computeD2F(unsigned int j_var, unsigned int k_var);
  virtual Real computeD3F(unsigned int j_var, unsigned int k_var, unsigned int l_var);

private:
  /// Coupled variable value for the concentration \f$ c \f$.
  const VariableValue & _c;
  unsigned int _c_var;
};
