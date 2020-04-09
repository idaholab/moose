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
 * Material class that provides the free energy of an ideal gas with the expression builder
 * and uses automatic differentiation to get the derivatives.
 */
class GasFreeEnergyBase : public DerivativeParsedMaterialHelper, public ExpressionBuilder
{
public:
  static InputParameters validParams();

  GasFreeEnergyBase(const InputParameters & parameters);

protected:
  /// Coupled variable value for the Temperature
  const EBTerm _T;

  /// Coupled variable value for the concentration \f$ c \f$.
  const EBTerm _c;

  /// lattice site volume
  const Real _omega;

  /// gas molecule mass in eV*s^2/Ang^2
  const Real _m;

  ///@{ physical constants
  const Real _h;
  const Real _kB;
  ///@}

  /// gas number density n = N/V = c/Omega (where Omega is the lattice site volume)
  const EBTerm _n;

  /// quantum concentration
  const EBTerm _nq;
};
