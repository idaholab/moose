//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeFunctionMaterialBase.h"

class Function;

/**
 * Compute free energy and chemical potentials from user supplied MooseFunctions
 */
class CoupledValueFunctionFreeEnergy : public DerivativeFunctionMaterialBase
{
public:
  static InputParameters validParams();

  CoupledValueFunctionFreeEnergy(const InputParameters & parameters);

  void initialSetup() override;

protected:
  void computeProperties() override;

  const Function * _free_energy_function;

  const std::vector<FunctionName> _chemical_potential_names;
  std::vector<const Function *> _chemical_potential_functions;

  using FunctionMaterialBase::_nargs;
};
