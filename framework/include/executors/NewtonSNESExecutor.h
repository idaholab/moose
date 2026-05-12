//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SNESExecutor.h"
#include "MooseTypes.h"

#include <map>
#include <memory>
#include <utility>

namespace libMesh
{
template <typename>
class PetscMatrix;
}

/**
 * Executor implementing a Newton-type outer solve (SNESEWTONLS) for one or more libMesh
 * nonlinear systems.
 *
 * Three code paths are selected at run() time:
 *   Case 1: nonlinear_system_name is supplied -- solve only that system via _fe_problem.solve().
 *   Case 2: no name, single NL system on problem -- equivalent to Case 1 with system 0.
 *   Case 3: no name, multiple NL systems -- create a combined outer SNES with VecNest/MatNest
 *           over all systems.  nl_preconditioning is required in this path.
 */
class NewtonSNESExecutor : public SNESExecutor
{
public:
  static InputParameters validParams();
  NewtonSNESExecutor(const InputParameters & params);
  virtual ~NewtonSNESExecutor();

  virtual Result run() override;

protected:
  virtual void setupSNES() override;

private:
  /// System number to solve for Cases 1/2.  -1 means not yet determined.
  int _fixed_sys_num = -1;

  /// True when Case 3 (multi-system combined solve) is active.
  bool _multi_system = false;

  // Case 3 storage -----------------------------------------------------------

  /// VecNest wrapping per-system solution Vecs.  Built once in setupSNES().
  Vec _vec_sol = nullptr;

  /// PETSc MatNest holding all (i,j) Jacobian sub-blocks.
  Mat _mat_nest = nullptr;

  /// Off-diagonal Jacobian blocks J_ij (i != j).
  std::map<std::pair<unsigned int, unsigned int>,
           std::unique_ptr<libMesh::PetscMatrix<libMesh::Number>>>
      _off_diag_mats;

  /// Matrix tag IDs for each off-diagonal block.
  std::map<std::pair<unsigned int, unsigned int>, TagID> _off_diag_tags;

  // Case 3 helpers -----------------------------------------------------------

  void allocateOffDiagMats();
  void assembleOffDiagJacobian();
  void buildMatNest();

  static void registerFieldSplitIS(SNES snes, unsigned int n_sys);

  static PetscErrorCode outerResidualCallback(SNES snes, Vec x, Vec f, void * ctx);
  static PetscErrorCode outerJacobianCallback(SNES snes, Vec x, Mat A, Mat P, void * ctx);
};
