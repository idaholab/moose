//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "libmesh/sparsity_pattern.h"

/**
 * Preallocates the (LM, disp_k) cross-node Jacobian entries within each
 * lower-d contact element.  Without this, MOOSE's default sparsity
 * computation preallocates only the same-node (LM, disp) entries (which
 * RigidBodyNodalNCPKernel writes on the gap branch), and
 * RigidBodyNormalMechanicalContact's cross-node (LM_row, disp_col) writes
 * on the lower-d element force PETSc to malloc on every Jacobian assembly.
 *
 * Registers via SystemBase::addExtraSparsityCallback() so it coexists with
 * MOOSE's own extra-sparsity function on the nonlinear-system DofMap without
 * tripping libMesh's "both function *and* object slot set" warning.
 */
class RigidBodyContactSparsity : public GeneralUserObject
{
public:
  static InputParameters validParams();
  RigidBodyContactSparsity(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

private:
  /// Callback body registered with SystemBase::addExtraSparsityCallback().
  void applyExtraSparsity(libMesh::SparsityPattern::Graph & sparsity,
                          std::vector<libMesh::dof_id_type> & n_nz,
                          std::vector<libMesh::dof_id_type> & n_oz);

  const unsigned int _lm_var_num;
  const unsigned int _ndisp;
  std::vector<unsigned int> _disp_var_num;
};
