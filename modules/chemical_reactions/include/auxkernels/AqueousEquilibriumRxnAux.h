/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef AQUEOUSEQUILIBRIUMRXNAUX_H
#define AQUEOUSEQUILIBRIUMRXNAUX_H

#include "AuxKernel.h"

// Forward Declarations
class AqueousEquilibriumRxnAux;

template <>
InputParameters validParams<AqueousEquilibriumRxnAux>();

/**
 * Define the AuxKernel for the output of equilibrium species concentrations
 * according to mass action law.
 */
class AqueousEquilibriumRxnAux : public AuxKernel
{
public:
  AqueousEquilibriumRxnAux(const InputParameters & parameters);

  virtual ~AqueousEquilibriumRxnAux() {}

protected:
  virtual Real computeValue() override;

  /// Equilibrium constant
  const Real _log_k;

  /// Stoichiometric coefficients for coupled primary species
  const std::vector<Real> _sto_v;

  /// Coupled primary species
  std::vector<const VariableValue *> _vals;
};

#endif // AQUEOUSEQUILIBRIUMRXNAUX_H
