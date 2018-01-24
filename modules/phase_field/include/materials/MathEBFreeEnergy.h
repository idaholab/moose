//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATHEBFREEENERGY_H
#define MATHEBFREEENERGY_H

#include "DerivativeParsedMaterialHelper.h"
#include "ExpressionBuilder.h"

// Forward Declarations
class MathEBFreeEnergy;

template <>
InputParameters validParams<MathEBFreeEnergy>();

/**
 * Material class that creates the math free energy with the expression builder
 * and uses automatic differentiation to get the derivatives.
 * \f$ F = \frac14 (1 + c)^2 (1 - c)^2 \f$.
 */
class MathEBFreeEnergy : public DerivativeParsedMaterialHelper, public ExpressionBuilder
{
public:
  MathEBFreeEnergy(const InputParameters & parameters);

protected:
  /// Coupled variable value for the concentration \f$ c \f$.
  EBTerm _c;
};

#endif // MATHEBFREEENERGY_H
