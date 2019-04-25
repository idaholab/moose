//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"
#include "DerivativeMaterialPropertyNameInterface.h"

// Forward Declarations
template <ComputeStage>
class ADTestDerivativeFunction;

declareADValidParams(ADTestDerivativeFunction);

/**
 * Material class that creates the math free energy and its derivatives
 * for use with ADSplitCHParsed. \f$ F = \frac14(1 + c)^2(1 - c)^2 \f$.
 */
template <ComputeStage compute_stage>
class ADTestDerivativeFunction : public ADMaterial<compute_stage>,
                                 public DerivativeMaterialPropertyNameInterface
{
public:
  ADTestDerivativeFunction(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  enum class FunctionEnum
  {
    F1,
    F2
  } _function;

  /// Coupled variable value for the order parameter
  std::vector<const ADVariableValue *> _op;

  /// property name
  const MaterialPropertyName _f_name;

  /// function value
  ADMaterialProperty(Real) & _prop_F;

  /// function value derivative
  std::vector<ADMaterialProperty(Real) *> _prop_dFdop;

  usingMaterialMembers;
};

