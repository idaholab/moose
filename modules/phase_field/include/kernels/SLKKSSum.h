//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "JvarMapInterface.h"

/**
 * Enforce the sum of sublattice concentrations to a given phase concentration.
 * This is for test simulations to equilibrate the internal degrees of freedom
 * in a phase with multiple sublattices. The total concentration is prescribed
 * by an AuxVariable.
 * D. Schwen et al. https://doi.org/10.1016/j.commatsci.2021.110466
 */
class SLKKSSum : public JvarMapKernelInterface<Kernel>
{
public:
  static InputParameters validParams();

  SLKKSSum(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  ///@{ sublattice A variables
  unsigned int _ncs;
  std::vector<const VariableValue *> _cs;
  std::vector<Real> _a_cs;
  const JvarMap & _cs_map;
  ///@}

  /// sublattice fraction for the sublattice B concentration represented by the kernel variable
  Real _a_u;

  /// AuxVariable to hold the target sum
  const VariableValue & _target;
};
