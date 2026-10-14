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
#include "LowerDIntegratedBC.h"

/**
 * Traction on one displacement component from a rigid-body frictionless contact.
 * Companion to RigidBodyNodalNCPKernel (which handles the LM row).
 *
 *   R_{u_k}(i) += -lambda * n_k(x + u) * phi_i
 *
 * Off-diagonal:  d R_{u_k} / d lambda_j = -n_k * phi_i * phi_lambda_j.
 * With `finite_strain = true`, adds d R_{u_k} / d u_l = -lambda * H_kl * phi_l_j * phi_i.
 */
class RigidBodyNormalMechanicalContact : public LowerDIntegratedBC
{
public:
  static InputParameters validParams();
  RigidBodyNormalMechanicalContact(const InputParameters &);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int) override;
  virtual Real computeLowerDQpResidual() override { return 0; }
  virtual Real computeLowerDQpJacobian(Moose::ConstraintJacobianType type) override;
  virtual Real computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType,
                                              const MooseVariableFEBase &) override
  {
    return 0;
  }

private:
  Point deformedPoint() const;
  unsigned int dispIndex(unsigned int) const;
  /// Cached contactor query at the current QP's deformed point.  Recomputes
  /// lazily when the query point changes; the cache is a per-thread mutable
  /// member (IntegratedBCs are cloned per thread).
  const LevelSetContactor::Query & query() const;

  const LevelSetContactor & _contactor;
  const unsigned int _component;
  const unsigned int _ndisp;
  std::vector<const VariableValue *> _disp;
  std::vector<unsigned int> _disp_num;
  const bool _finite_strain;

  mutable Point _cache_pt;
  mutable LevelSetContactor::Query _cache_q;
  mutable bool _cache_valid = false;
};
