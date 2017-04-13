/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef PETSCDMMOOSE_H
#define PETSCDMMOOSE_H

// This only works with petsc-3.3 and above.
#include "libmesh/petsc_macro.h"

#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3, 3, 0)

// PETSc includes
#include <petscdm.h>
#define DMMOOSE "moose"

// C++ includes
#include <vector>
#include <set>
#include <string>

// Forward declarations
class NonlinearSystemBase;

extern PetscErrorCode DMMooseRegisterAll();
extern PetscErrorCode DMCreateMoose(MPI_Comm, NonlinearSystemBase &, DM *);
extern PetscErrorCode DMMooseReset(DM);
extern PetscErrorCode DMMooseSetNonlinearSystem(DM, NonlinearSystemBase &);
extern PetscErrorCode DMMooseGetNonlinearSystem(DM, NonlinearSystemBase *&);
extern PetscErrorCode DMMooseGetBlocks(DM, std::vector<std::string> &);
extern PetscErrorCode DMMooseGetVariables(DM, std::vector<std::string> &);
extern PetscErrorCode DMMooseGetSides(DM, std::set<std::string> &);
extern PetscErrorCode DMMooseGetUnSides(DM, std::set<std::string> &);
extern PetscErrorCode
DMMooseGetContacts(DM, std::vector<std::pair<std::string, std::string>> &, std::vector<bool> &);
extern PetscErrorCode
DMMooseGetUnContacts(DM, std::vector<std::pair<std::string, std::string>> &, std::vector<bool> &);
extern PetscErrorCode DMMooseSetBlocks(DM, const std::vector<std::string> &);
extern PetscErrorCode DMMooseSetVariables(DM, const std::vector<std::string> &);
extern PetscErrorCode DMMooseSetSides(DM, const std::set<std::string> &);
extern PetscErrorCode DMMooseSetUnSides(DM, const std::set<std::string> &);
extern PetscErrorCode DMMooseSetContacts(DM,
                                         const std::vector<std::pair<std::string, std::string>> &,
                                         const std::vector<bool> &);
extern PetscErrorCode DMMooseSetUnContacts(DM,
                                           const std::vector<std::pair<std::string, std::string>> &,
                                           const std::vector<bool> &);
extern PetscErrorCode DMMooseSetSplitNames(DM, const std::vector<std::string> &);
extern PetscErrorCode DMMooseGetSplitNames(DM, const std::vector<std::string> &);
extern PetscErrorCode DMMooseSetSplitVars(DM, const std::string &, const std::set<std::string> &);
extern PetscErrorCode DMMooseGetSplitVars(DM, const std::string &, std::set<std::string> &);
extern PetscErrorCode DMMooseSetSplitBlocks(DM, const std::string &, const std::set<std::string> &);
extern PetscErrorCode DMMooseGetSplitBlocks(DM, const std::string &, std::set<std::string> &);
extern PetscErrorCode DMMooseSetSplitSides(DM, const std::string &, const std::set<std::string> &);
extern PetscErrorCode DMMooseGetSplitSides(DM, const std::string &, std::set<std::string> &);
extern PetscErrorCode SNESUpdateDMMoose(SNES snes, PetscInt iteration);

#endif // #if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
#endif // #ifdef PETSCDMMOOSE_H
