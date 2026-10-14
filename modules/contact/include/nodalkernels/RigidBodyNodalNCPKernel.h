//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LevelSetContactor.h"
#include "NodalKernel.h"

/**
 * Rigid-body frictionless contact - node-wise NCP enforced against an
 * analytic level-set contactor. Applied at each Lagrange-multiplier DoF
 * living on the deformable contact sideset's lower-d block.
 *
 *   R_i = min( lambda_i,  c * g_LS(x_i + u_i) )
 *
 * Complementarity is completed by PETSc `SNESVINEWTONSSLS` + `ConstantBounds`
 * enforcing lambda >= 0. The nodal min-NCP puts a nonzero {0, 1} on the MOOSE
 * assembled Jacobian's LM diagonal (same structural benefit that mortar's
 * `enforceConstraintOnDof` provides). No mortar segment mesh, no dual-basis
 * integration, no AD.
 *
 * Jacobian:
 *   lambda-branch  (lambda <= c * g):  dR/dlambda = 1,  dR/ddisp_k = 0
 *   g-branch  (c * g < lambda):  dR/dlambda = 0,  dR/ddisp_k = c * n_k(x + u)
 *
 * Companion class: RigidBodyNormalMechanicalContact applies the
 * -lambda * n * phi_test traction on the coupled displacement equations.
 */
class RigidBodyNodalNCPKernel : public NodalKernel
{
public:
  static InputParameters validParams();
  RigidBodyNodalNCPKernel(const InputParameters &);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  Point deformedNode() const;
  /// Return the cached contactor query for the current deformed node.
  /// Recomputes lazily when the query point changes; the cache is a per-
  /// thread mutable member (NodalKernels are cloned per thread, so mutable
  /// state on the instance is safe).
  const LevelSetContactor::Query & query() const;

  const LevelSetContactor & _contactor;
  const Real _c;
  const unsigned int _ndisp;
  std::vector<const VariableValue *> _disp;
  std::vector<unsigned int> _disp_num;

  mutable Point _cache_pt;
  mutable LevelSetContactor::Query _cache_q;
  mutable bool _cache_valid = false;
};
