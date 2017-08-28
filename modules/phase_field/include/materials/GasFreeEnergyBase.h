/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GASFREEENERGYBASE_H
#define GASFREEENERGYBASE_H

#include "DerivativeParsedMaterialHelper.h"
#include "ExpressionBuilder.h"

// Forward Declarations
class GasFreeEnergyBase;

template <>
InputParameters validParams<GasFreeEnergyBase>();

/**
 * Material class that provides the free energy of an ideal gas with the expression builder
 * and uses automatic differentiation to get the derivatives.
 */
class GasFreeEnergyBase : public DerivativeParsedMaterialHelper, public ExpressionBuilder
{
public:
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

#endif // IDEALGASFREEENERGY_H
