//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialPropertyNameInterface.h"

/**
 * Material class that creates the math free energy and its derivatives
 * for use with ADSplitCHParsed. \f$ F = \frac14(1 + c)^2(1 - c)^2 \f$.
 */
class ADTestDerivativeFunction : public Material, public DerivativeMaterialPropertyNameInterface
{
public:
  static InputParameters validParams();

  ADTestDerivativeFunction(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  enum class FunctionEnum
  {
    F1,
    F2,
    F3
  } _function;

  /// Coupled variable value for the order parameter
  const std::vector<const ADVariableValue *> _op;

  /// property name
  const MaterialPropertyName _f_name;

  /// function value
  ADMaterialProperty<Real> & _prop_F;

  /// function value derivative
  std::vector<ADMaterialProperty<Real> *> _prop_dFdop;
};
