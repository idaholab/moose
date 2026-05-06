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

#include "libmesh/petsc_macro.h"
#ifdef LIBMESH_HAVE_PETSC
#include <petscsnes.h>
#endif

class FEProblemBase;

/**
 * Sets up a PETSc SNESSHELL as a nonlinear preconditioner (NPC) that applies inner
 * SNES solves for a subset of systems before each outer Newton step.  This implements
 * nonlinear elimination (NLE) when only a strict subset of systems is listed; listing
 * all systems reproduces nonlinear Gauss-Seidel.
 */
class NonlinearPreconditioning : public MooseObject, public PerfGraphInterface
{
public:
  static InputParameters validParams();
  NonlinearPreconditioning(const InputParameters & params);
  ~NonlinearPreconditioning();

  /// Create the NPC SNES shell.  Called from FEProblemBase::initialSetup().
  void initialSetup();

  /// Wire the NPC onto the outer SNES for system sys_num.  Must be called before each solve
  /// because libMesh destroys and recreates the SNES object at the end of every solve.
  void wireToSNES(unsigned int sys_num);

  /// System numbers solved as inner NPC (not driven directly by FEProblemSolve).
  const std::vector<unsigned int> & innerSysNums() const { return _inner_sys_nums; }

protected:
  FEProblemBase & _fe_problem;

  /// Nonlinear system indices handled by the inner NPC solve.
  std::vector<unsigned int> _inner_sys_nums;

#ifdef LIBMESH_HAVE_PETSC
  SNES _npc_snes = nullptr;

  /// Create the SNESSHELL NPC object.
  void setupNPC();

  /// PETSc SNESSHELL solve callback: runs _fe_problem.solve() for each inner system.
  static PetscErrorCode npcShellSolve(SNES snes, Vec x);
#endif
};
