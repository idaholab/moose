//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Executor.h"
#include "MooseTypes.h"
#include "PetscSupport.h"

#include "libmesh/petsc_macro.h"
#include <petscsnes.h>

class FEProblemBase;
class SNESNPCExecutor;

/**
 * Abstract base for Executor-derived classes that own a PETSc SNES object.
 *
 * Derived classes implement setupSNES() to create and configure their SNES, and
 * run() to perform the solve or preconditioning step.  A SNESExecutor can be used
 * as the outer solver (its run() is called directly) or as a nonlinear preconditioner
 * (its SNES is retrieved via getSNES() and attached to another SNES via SNESSetNPC).
 *
 * The residual Vec _vec_func is allocated only when the executor is the outer solver.
 * When acting as an NPC the outer SNES manages the residual.
 */
class SNESExecutor : public Executor
{
public:
  static InputParameters validParams();
  SNESExecutor(const InputParameters & params);
  virtual ~SNESExecutor();

  /// Return the owned SNES
  virtual SNES getSNES();

protected:
  FEProblemBase & _fe_problem;

  /// Owned PETSc SNES. If this wraps a libMesh nonlinar solve this should always be nullptr
  SNES _snes = nullptr;

  /// VecNest wrapping per-system solution Vecs.  Built once in setupSNES().
  Vec _vec_sol = nullptr;

  /// Combined residual Vec. VecNest of per-system RHS Vecs for multiple systems
  Vec _vec_func = nullptr;

  /// Optional nonlinear preconditioner executor.  Non-null when nl_preconditioning is set.
  SNESNPCExecutor * _npc_executor = nullptr;

  /// Create and configure the SNES.  Called lazily on the first run().
  virtual void setupSNES() = 0;

  /// Whether setupSNES() has already been called.
  bool _snes_setup_done = false;
};
