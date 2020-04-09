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
 * Material class that creates regular solution free energy with the expression builder
 * and uses automatic differentiation to get the derivatives
 * \f$ F = \frac14 \omega c(1 - c) + k_bT (c\log c + (1 - c)\log(1 - c))\f$.
 */
class RegularSolutionFreeEnergy : public DerivativeParsedMaterialHelper, public ExpressionBuilder
{
public:
  static InputParameters validParams();

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
