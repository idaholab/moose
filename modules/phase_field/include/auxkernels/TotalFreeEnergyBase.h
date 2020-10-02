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

// Forward Declarations

/**
 * Total free energy (both the bulk and gradient parts), where the bulk free energy has been defined
 * in a material and called f_name
 */
class TotalFreeEnergyBase : public AuxKernel
{
public:
  static InputParameters validParams();

  TotalFreeEnergyBase(const InputParameters & parameters);

protected:
  virtual Real computeValue() = 0;

  /// Coupled interface variables
  unsigned int _nvars;
  const std::vector<const VariableValue *> _vars;
  const std::vector<const VariableGradient *> _grad_vars;

  /// Gradient free energy prefactor kappa
  std::vector<MaterialPropertyName> _kappa_names;
  unsigned int _nkappas;

  /// Additional free energy contribution
  const VariableValue & _additional_free_energy;
};
