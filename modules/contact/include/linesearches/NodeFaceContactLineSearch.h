//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_common.h"
#include "libmesh/petsc_macro.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include <petscsnes.h>

#include "ContactLineSearchBase.h"

using namespace libMesh;

/**
 * Node-face contact line search (based on the Petsc LineSearchShell): a lambda-cut algorithm that
 * repeatedly halves the Newton step length until the residual norm decreases, tracking the active
 * mechanical contact set across iterations.
 *
 * When the contact set is changing, the user may optionally use a looser linear tolerance set by
 * the `contact_line_search_ltol` Executioner parameter (mapped to `contact_ltol` here). Then when
 * the contact set is changing during the beginning of the Newton solve, unnecessary computational
 * expense is avoided. Then when the contact set is resolved late in the Newton solve, the linear
 * tolerance will return to the finer tolerance set through the traditional `l_tol` parameter.
 *
 * The number of allowed lambda cuts is controlled through the `contact_line_search_allowed_lambda_cuts`
 * Executioner parameter.
 */
class NodeFaceContactLineSearch : public ContactLineSearchBase
{
public:
  static InputParameters validParams();

  NodeFaceContactLineSearch(const InputParameters & parameters);

  virtual void lineSearch() override;

  void printContactInfo(const std::set<dof_id_type> & contact_set);
  void insertSet(const std::set<dof_id_type> & mech_set);
  virtual void reset();

protected:
  std::set<dof_id_type> _current_contact_state;
  std::set<dof_id_type> _old_contact_state;

  Real _user_ksp_rtol;
  bool _user_ksp_rtol_set;

  Real _contact_lambda;

  /// The number of times lambda is allowed to get cut
  unsigned _allowed_lambda_cuts;
};
