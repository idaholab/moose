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
class ADMathFreeEnergy;

declareADValidParams(ADMathFreeEnergy);

/**
 * Material class that creates the math free energy and its derivatives
 * for use with ADSplitCHParsed. \f$ F = \frac14(1 + c)^2(1 - c)^2 \f$.
 */
template <ComputeStage compute_stage>
class ADMathFreeEnergy : public ADMaterial<compute_stage>,
                         public DerivativeMaterialPropertyNameInterface
{
public:
  ADMathFreeEnergy(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Coupled variable value for the concentration \f$ c \f$.
  const ADVariableValue & _c;

  /// property name
  const MaterialPropertyName _f_name;

  /// function value
  ADMaterialProperty(Real) & _prop_F;

  /// function value derivative
  ADMaterialProperty(Real) & _prop_dFdc;

  usingMaterialMembers;
};

