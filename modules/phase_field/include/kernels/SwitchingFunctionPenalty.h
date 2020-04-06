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
#include "DerivativeMaterialInterface.h"

/**
 * SwitchingFunctionPenalty is a constraint kernel adds a penalty
 * to each order parameter to
 * enforce \f$ \sum_n h_i(\eta_i) \equiv 1 \f$.
 */
class SwitchingFunctionPenalty : public DerivativeMaterialInterface<Kernel>
{
public:
  static InputParameters validParams();

  SwitchingFunctionPenalty(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  /// Switching function names
  std::vector<MaterialPropertyName> _h_names;
  unsigned int _num_h;

  /// Switching functions and their drivatives
  std::vector<const MaterialProperty<Real> *> _h, _dh;
  const MaterialProperty<Real> * _d2h;

  /// Penalty pre-factor
  const Real _penalty;

  /// number of non-linear variables in the problem
  const unsigned int _number_of_nl_variables;

  /// eta index for the j_vars in the jacobian computation
  std::vector<int> _j_eta;

  /// Index of the eta this kernel is operating on
  int _a;
};
