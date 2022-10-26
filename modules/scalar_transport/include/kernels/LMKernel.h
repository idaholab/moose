//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * Base class for use when adding Pressure-Stabilized Petrov-Galerkin type stabilization (e.g. a
 * diagonal) to Lagrange multiplier constraints. This class will add its residual to both the primal
 * equation and the Lagrange multiplier constraint equation
 */
class LMKernel : public ADKernelValue
{
public:
  LMKernel(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  void computeResidual() override;
  void computeResidualsForJacobian() override;

  /// The Lagrange multiplier variable
  MooseVariable & _lm_var;

  /// The values of the Lagrange multiplier variable at quadrature points
  const ADVariableValue & _lm;

  /// The values of the Lagrange multiplier test functions at quadrature points
  const VariableTestValue & _lm_test;

  /// The sign (either +1 or -1) applied to this object's residual when adding
  /// to the Lagrange multiplier constraint equation. The sign should be chosen
  /// such that the diagonals for the LM block of the matrix are positive
  const Real _lm_sign;

private:
  const std::vector<dof_id_type> & dofIndices() const override { return _all_dof_indices; }

  /// The union of the primary var dof indices as well as the LM dof indices
  std::vector<dof_id_type> _all_dof_indices;
};
