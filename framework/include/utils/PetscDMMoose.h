//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/petsc_macro.h"

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

/**
 * Create a MOOSE DM
 * @param comm The communicator that the DM should use
 * @param nl The nonlinear system context that the DM is associated with
 * @param dm_name A name to associate with the DM
 * @param dm A pointer to the PETSc DM
 */
extern PetscErrorCode
DMCreateMoose(MPI_Comm comm, NonlinearSystemBase & nl, const std::string & dm_name, DM * dm);

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
