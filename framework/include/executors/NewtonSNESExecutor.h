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
  virtual SNES getSNES() override;

  /**
   * @returns the libMesh system corresponding to the index \p i
   */
  libMesh::System & getSystem();

protected:
  virtual void setupSNES() override;

private:
  /// The nonlinear systems this SNES targets
  std::vector<unsigned int> _nl_sys_nums;

  /// PETSc MatNest holding all (i,j) Jacobian sub-blocks.
  Mat _mat_nest = nullptr;

  struct MatData
  {
    std::unique_ptr<libMesh::PetscMatrix<libMesh::Number>> mat;
    TagID tag;
  };

  /// Off-diagonal Jacobian blocks J_ij (i != j).
  std::map<std::pair<unsigned int, unsigned int>, MatData> _off_diag_mats;

  // Multi-system helpers -----------------------------------------------------------

  void allocateOffDiagMats();
  void assembleOffDiagJacobian();
  void buildMatNest();

  static PetscErrorCode outerResidualCallback(SNES snes, Vec x, Vec f, void * ctx);
  static PetscErrorCode outerJacobianCallback(SNES snes, Vec x, Mat A, Mat P, void * ctx);

  //
  // shell machinery for an outer SNES
  //

  /// Shell matrix representing multiplication of the linearized nonlinear
  /// preconditioner and the full Jacobian. I think of it as representing B*A,
  /// where B is what PETSc typically uses to denote the preconditioner (*not*
  /// what MOOSE calls the preconditioning matrix P, but something more like
  /// P^{-1}). The nonlinear preconditioner supplies the exact block application
  /// used by this shell matrix.
  Mat _jac_shell = nullptr;

  /**
   * Shell routine associated with the _jac_shell that will forward to the nonlinear preconditioner
   * for applying the preconditioning at the linearized operator level and for applying our global
   * MatNest Jacobian
   */
  static PetscErrorCode shellMatMult(Mat m, Vec X, Vec Y);
};
