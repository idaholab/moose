//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LineSearch.h"
#include "MooseEnum.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/petsc_macro.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include <petscsnes.h>

using namespace libMesh;

class FEProblem;

/**
 * Shared infrastructure for the PETSc-based contact line searches (node-face and mortar):
 * creation/configuration of a secondary backing SNESLineSearch, and the optional linear-tolerance
 * loosening applied while the contact set is changing.
 *
 * When the contact set is changing, the user may optionally use a looser linear tolerance set by
 * the `contact_ltol` parameter. Then when the contact set is changing during the beginning of the
 * Newton solve, unnecessary computational expense is avoided. Then when the contact set is
 * resolved late in the Newton solve, the linear tolerance will return to the finer tolerance set
 * through the traditional `l_tol` parameter.
 *
 * The `backing_line_search` parameter selects the standard PETSc SNES line search type (`basic`,
 * `bt`, `l2`, or `cp`) a subclass uses as the backing algorithm for its constraint-set-aware
 * wrapper (see setupBackingLineSearch()).
 */
class ContactLineSearchBase : public LineSearch
{
public:
  static InputParameters validParams();

  ContactLineSearchBase(const InputParameters & parameters);
  ~ContactLineSearchBase();

protected:
  /**
   * Create and configure the secondary backing SNESLineSearch on first use, and rebind it to the
   * solver's current SNES on every call. It is a standalone SNESLineSearch object (not another
   * shell) set to the user-selected type with its own "contact_backing_" PETSc options prefix, so
   * users can tune it with native PETSc options. The rebinding is necessary because the solver's
   * SNES may be destroyed and recreated between solves.
   */
  void setupBackingLineSearch();

  PetscNonlinearSolver<Real> * _solver;

  /// The secondary backing PETSc line search; null until setupBackingLineSearch() is first called
  SNESLineSearch _backing_petsc_line_search;

  /// What the linear tolerance should be while the contact state is changing
  Real _contact_ltol;

  /// Whether to modify the linear tolerance
  bool _affect_ltol;

  /// The standard PETSc SNES line search type (basic, bt, l2, cp) a subclass backs itself with
  MooseEnum _backing_line_search;
};
