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

/**
 * validParams returns the parameters that this Kernel accepts / needs
 * The actual body of the function MUST be in the .C file.
 */
template<>
InputParameters validParams<AqueousEquilibriumRxnAux>();

/**
 * Define the AuxKernel for the output of equilibrium species concentrations
 * according to mass action law.
 */
class AqueousEquilibriumRxnAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  AqueousEquilibriumRxnAux(const InputParameters & parameters);

  virtual ~AqueousEquilibriumRxnAux() {}

protected:
  /**
   * Conputes the equilibrium sepecies concentration.
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
