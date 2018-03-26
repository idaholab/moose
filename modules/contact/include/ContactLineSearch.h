//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONTACTLINESEARCH_H
#define CONTACTLINESEARCH_H

#include "libmesh/petsc_nonlinear_solver.h"

#ifdef LIBMESH_HAVE_PETSC

#include "libmesh/parallel_object.h"
#include <petscsnes.h>

#include "ConsoleStreamInterface.h"

using namespace libMesh;

class FEProblemBase;

class ContactLineSearch : public PetscNonlinearSolver<Real>::ComputeLineSearchObject,
                          public ConsoleStreamInterface,
                          public ParallelObject
{
public:
  ContactLineSearch(FEProblemBase & fe_problem,
                    MooseApp & app,
                    size_t allowed_lambda_cuts,
                    Real contact_ltol,
                    bool affect_ltol);

  /**
   * The custom linesearch implementation method
   */
  virtual void linesearch(SNESLineSearch linesearch) override;

  /**
   * The current contact state
   */
  std::set<dof_id_type> * contact_state() { return &_current_contact_state; }

  /**
   * The old contact state
   */
  std::set<dof_id_type> * old_contact_state() { return &_old_contact_state; }

  /**
   * Method for printing the contact information
   */
  void printContactInfo();

  /**
   * writeable reference to number of non-linear iterations
   */
  size_t & nl_its() { return _nl_its; }

  /**
   * read-only reference to number of non-linear iterations
   */
  const size_t & nl_its() const { return _nl_its; }

protected:
  /// Reference to the finite element problem
  FEProblemBase & _fe_problem;

  /// The current contact set
  std::set<dof_id_type> _current_contact_state;
  /// The old contact set
  std::set<dof_id_type> _old_contact_state;

  /// number of non-linear iterations
  size_t _nl_its;

  /// the linear tolerance set by the user in the input file
  PetscReal _user_ksp_rtol;
  /// Whether the user linear tolerance has been set yet in this object
  bool _user_ksp_rtol_set;

  /// The multiplier of the newton step
  PetscReal _contact_lambda;

  /// How many times the linsearch is allowed to cut lambda
  size_t _allowed_lambda_cuts;

  /// What the linear tolerance should be while the contact state is changing
  Real _contact_ltol;

  /// Whether to modify the linear tolerance
  bool _affect_ltol;
};

#endif
#endif
