/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TOTALCONCENTRATIONAUX_H
#define TOTALCONCENTRATIONAUX_H

#include "AuxKernel.h"

class TotalConcentrationAux;

template <>
InputParameters validParams<TotalConcentrationAux>();

/**
 * Computes the total concentration of given primary species, including its free
 * concentration and its stoichiometric contribution to all secondary equilibrium
 * species that it is involved in
 */
class TotalConcentrationAux : public AuxKernel
{
public:
  TotalConcentrationAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Primary species that this AuxKernel acts on
  const VariableValue & _primary_species;
  /// Stoichiometric coefficients for primary species in coupled secondary species
  const std::vector<Real> _sto_v;
  /// Coupled secondary species concentration
  std::vector<const VariableValue *> _secondary_species;
};

#endif // TOTALCONCENTRATIONAUX_H
