//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "PerfGraphInterface.h"
#include "MooseTypes.h"

#include "libmesh/petsc_macro.h"
#include <petscsnes.h>
#include <petscmat.h>

#include <map>
#include <memory>
#include <utility>

namespace libMesh
{
template <typename>
class PetscMatrix;
}

class FEProblemBase;

/**
 * Abstract base for nonlinear preconditioners (NPC) in multi-system solves.
 * Derived classes implement setupNPC() to create and configure the inner SNES.
 *
 * This class manages off-diagonal Jacobian blocks J_ij (i != j) and assembles them
 * into a PETSc MatNest outer Jacobian operator so that the outer KSP can apply
 * block-structured preconditioners such as fieldsplit.
 */
class NonlinearPreconditioning : public MooseObject, public PerfGraphInterface
{
public:
  static InputParameters validParams();
  NonlinearPreconditioning(const InputParameters & params);
  ~NonlinearPreconditioning();

  /// Create the NPC SNES shell and allocate off-diagonal Jacobian blocks.
  /// Called from FEProblemBase::initialSetup().
  void initialSetup();

  /// Wire the NPC onto the outer SNES for system sys_num and set up the MatNest Jacobian.
  /// Must be called before each solve because libMesh destroys and recreates the SNES object.
  void wireToSNES(unsigned int sys_num);

  /// System numbers solved as inner NPC (not driven directly by FEProblemSolve).
  const std::vector<unsigned int> & innerSysNums() const { return _inner_sys_nums; }

protected:
  FEProblemBase & _fe_problem;

  /// Nonlinear system indices handled by the inner NPC solve.
  std::vector<unsigned int> _inner_sys_nums;

  /// Outer SNES
  SNES _outer_snes = nullptr;
  /// Nonlinear preconditioner SNES
  SNES _npc_snes = nullptr;
  /// PETSc MatNest holding all (i,j) Jacobian sub-blocks.
  Mat _mat_nest = nullptr;

  /// Off-diagonal Jacobian blocks J_ij, indexed by (i, j) with i != j.
  std::map<std::pair<unsigned int, unsigned int>,
           std::unique_ptr<libMesh::PetscMatrix<libMesh::Number>>>
      _off_diag_mats;

  /// Matrix tag IDs for each off-diagonal block, used for tag-based routing.
  std::map<std::pair<unsigned int, unsigned int>, TagID> _off_diag_tags;

  /// Create and configure the inner NPC SNES object.  Called once from initialSetup().
  virtual void setupNPC() = 0;

  /// Allocate off-diagonal PETSc matrices and register them with the tag system.
  void allocateOffDiagMats();

  /// Assemble off-diagonal blocks J_ij (i != j) using the ISys/JSys context and
  /// a temporary matrix swap so existing kernels contribute via their normal matrix tags.
  void assembleOffDiagJacobian();

  /// (Re)build the MatNest from current diagonal and off-diagonal block matrices.
  void buildMatNest(unsigned int outer_sys_num);

  /// Register each system's IS with the outer PC for fieldsplit configuration.
  static void registerFieldSplitIS(SNES outer_snes, unsigned int n_sys);

  /// Outer SNES Jacobian callback: assembles all diagonal and off-diagonal blocks.
  static PetscErrorCode outerJacobianCallback(SNES snes, Vec x, Mat A, Mat P, void * ctx);
};
