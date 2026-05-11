//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NonlinearPreconditioning.h"

/**
 * Nonlinear Gauss-Seidel (NGS) preconditioning using PETSc SNESNGS.
 *
 * Sets up the NPC SNES as type SNESNGS and registers an NGS sweep function via
 * SNESSetNGS, producing -npc_snes_type ngs behavior.  Each sweep iterates over the
 * listed inner systems in order, calling _fe_problem.solve() for each.
 */
class NonlinearPreconditioningNGS : public NonlinearPreconditioning
{
public:
  static InputParameters validParams();
  NonlinearPreconditioningNGS(const InputParameters & params);

protected:
  void setupNPC() override;

#ifdef LIBMESH_HAVE_PETSC
  /// NGS sweep callback registered with SNESSetNGS.
  static PetscErrorCode ngsCallback(SNES snes, Vec x, Vec b, void * ctx);
#endif
};
