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
class SNESExecutor;

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

  /// Return the owned SNES for NPC wiring by a parent executor.
  SNES getSNES();

  /// Mark this executor as a nonlinear preconditioner for another executor.
  /// Called by the parent executor before getSNES().  Suppresses _vec_func allocation.
  void setIsNPC(bool v) { _is_npc = v; }

protected:
  FEProblemBase & _fe_problem;

  /// Owned PETSc SNES.  Null in the monolithic path of NewtonSNESExecutor before
  /// the first call to run() (the system SNES is used directly in that path).
  SNES _snes = nullptr;

  /// Combined residual Vec.  VecNest of per-system RHS Vecs in the multi-system path,
  /// the system's own RHS Vec in the monolithic path.
  /// Allocated only when _is_npc == false (outer solver role).
  Vec _vec_func = nullptr;

  /// True when this executor's SNES is attached as NPC to another executor's SNES.
  bool _is_npc = false;

  /// Optional nonlinear preconditioner executor.  Non-null when nl_preconditioning is set.
  SNESExecutor * _npc_executor = nullptr;

  /// Create and configure the SNES.  Called lazily on the first run().
  virtual void setupSNES() = 0;

  /// Whether setupSNES() has already been called.
  bool _snes_setup_done = false;
};
