/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef AQUEOUSEQUILIBRIUMRXNAUX_H
#define AQUEOUSEQUILIBRIUMRXNAUX_H

#include "AuxKernel.h"

//Forward Declarations
class AqueousEquilibriumRxnAux;

template<>
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
  /**
   * Computes the equilibrium sepecies concentration.
   * @return The concentration of an equilibrium species.
   */
  virtual Real computeValue();

  /// Equilibrium constant
  Real _log_k;

  /// Stochiometric coefficients for coupled primary species
  std::vector<Real> _sto_v;

  /// Coupled primary species
  std::vector<const VariableValue *>  _vals;
};

#endif //AQUEOUSEQUILIBRIUMRXNAUX_H
