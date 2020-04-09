//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowPermeabilityBase.h"

/**
 * Material designed to provide the permeability tensor which is calculated
 * from a tensor multiplied by a scalar:
 * k = k_ijk * k0
 * where k_ijk is a tensor providing the anisotropy, and k0 is a scalar
 * variable.
 */
class PorousFlowPermeabilityTensorFromVar : public PorousFlowPermeabilityBase
{
public:
  static InputParameters validParams();

  PorousFlowPermeabilityTensorFromVar(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  /// Permeability components
  const VariableValue & _perm;

  /// Tensor multiplier k_ijk
  const RealTensorValue _k_anisotropy;
};
