/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef REGULARSOLUTIONFREEENERGY_H
#define REGULARSOLUTIONFREEENERGY_H

#include "DerivativeParsedMaterialHelper.h"
#include "ExpressionBuilder.h"

// Forward Declarations
class RegularSolutionFreeEnergy;

template <>
InputParameters validParams<RegularSolutionFreeEnergy>();

/**
 * Material class that creates regular solution free energy with the expression builder
 * and uses automatic differentiation to get the derivatives
 * \f$ F = \frac14 \omega c(1 - c) + k_bT (c\log c + (1 - c)\log(1 - c))\f$.
 */
class RegularSolutionFreeEnergy : public DerivativeParsedMaterialHelper, public ExpressionBuilder
{
public:
  RegularSolutionFreeEnergy(const InputParameters & parameters);

protected:
  /// Coupled variable value for the concentration \f$ c \f$.
  EBTerm _c;

  /// Coupled temperature variable \f$ T \f$
  EBTerm _T;

  /// Prefactor
  const Real _omega;

  /// Boltzmann constant
  const Real _kB;
};

#endif // REGULARSOLUTIONFREEENERGY_H
