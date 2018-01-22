/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef AQUEOUSEQUILIBRIUMRXNAUX_H
#define AQUEOUSEQUILIBRIUMRXNAUX_H

#include "AuxKernel.h"

class AqueousEquilibriumRxnAux;

template <>
InputParameters validParams<AqueousEquilibriumRxnAux>();

/**
 * Calculates equilibrium species concentration according to the mass action law
 */
class AqueousEquilibriumRxnAux : public AuxKernel
{
public:
  AqueousEquilibriumRxnAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Equilibrium constant
  const Real _log_k;
  /// Stoichiometric coefficients of coupled primary species
  const std::vector<Real> _sto_v;
  /// Coupled primary species
  std::vector<const VariableValue *> _vals;
  /// Activity coefficients of coupled primary species
  std::vector<const VariableValue *> _gamma_v;
  /// Activity coefficient of equilibrium species
  const VariableValue & _gamma_eq;
};

#endif // AQUEOUSEQUILIBRIUMRXNAUX_H
