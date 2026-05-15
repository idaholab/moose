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

#include <vector>
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
  /// The nonlinear systems this SNES targets
  std::vector<unsigned int> _nl_sys_nums;

  /// PETSc MatNest holding all (i,j) Jacobian sub-blocks.
  Mat _mat_nest = nullptr;

  /// Off-diagonal Jacobian blocks J_ij (i != j).
  std::map<std::pair<unsigned int, unsigned int>,
           std::unique_ptr<libMesh::PetscMatrix<libMesh::Number>>>
      _off_diag_mats;

  /// Matrix tag IDs for each off-diagonal block.
  std::map<std::pair<unsigned int, unsigned int>, TagID> _off_diag_tags;

  // Multi-system helpers -----------------------------------------------------------

  void allocateOffDiagMats();
  void assembleOffDiagJacobian();
  void buildMatNest();

  static void registerFieldSplitIS(SNES snes, unsigned int n_sys);

  static PetscErrorCode outerResidualCallback(SNES snes, Vec x, Vec f, void * ctx);
  static PetscErrorCode outerJacobianCallback(SNES snes, Vec x, Mat A, Mat P, void * ctx);
};
