//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Computes the total concentration of given primary species, including its free
 * concentration and its stoichiometric contribution to all secondary equilibrium
 * species that it is involved in
 */
class TotalConcentrationAux : public AuxKernel
{
public:
  static InputParameters validParams();

  TotalConcentrationAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Primary species that this AuxKernel acts on
  const VariableValue & _primary_species;
  /// Stoichiometric coefficients for primary species in coupled secondary species
  const std::vector<Real> _sto_v;
  /// Coupled secondary species concentration
  const std::vector<const VariableValue *> _secondary_species;
};
