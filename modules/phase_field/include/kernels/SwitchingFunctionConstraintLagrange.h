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
#include "DerivativeMaterialInterface.h"
#include "NonlinearSystem.h"

/**
 * SwitchingFunctionConstraintLagrange is a constraint kernel that acts on the
 * lambda lagrange multiplier non-linear variables to
 * enforce \f$ \sum_n h_i(\eta_i) - \epsilon\lambda \equiv 1 \f$.
 */
class SwitchingFunctionConstraintLagrange
  : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  SwitchingFunctionConstraintLagrange(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  /// Switching function names
  std::vector<MaterialPropertyName> _h_names;

  /// number of switching functions
  unsigned int _num_h;

  /// Switching functions
  std::vector<const MaterialProperty<Real> *> _h;

  /// Switching function derivatives
  std::vector<std::vector<const MaterialProperty<Real> *>> _dh;

  /// map for getting the "etas" index from jvar
  const JvarMap & _eta_map;

  /// shift factor
  Real _epsilon;
};
