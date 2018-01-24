/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
