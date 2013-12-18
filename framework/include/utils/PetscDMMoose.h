#ifndef _PETSCDMMOOSE
#define _PETSCDMMOOSE

#include "libmesh/petsc_macro.h"
// This only works with petsc-3.3 and above.

#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
#include "NonlinearSystem.h"
#include <map>
#include <set>
#include <string>

#include <petscdm.h>
#define DMMOOSE "moose"

extern PetscErrorCode DMCreateMoose(MPI_Comm,NonlinearSystem&,DM*);
extern PetscErrorCode DMMooseReset(DM);
extern PetscErrorCode DMMooseSetNonlinearSystem(DM,NonlinearSystem&);
extern PetscErrorCode DMMooseGetNonlinearSystem(DM,NonlinearSystem*&);
extern PetscErrorCode DMMooseGetBlocks(DM,std::vector<std::string>&);
extern PetscErrorCode DMMooseGetVariables(DM,std::vector<std::string>&);
extern PetscErrorCode DMMooseGetSides(DM,std::set<std::string>&);
extern PetscErrorCode DMMooseGetUnSides(DM,std::set<std::string>&);
extern PetscErrorCode DMMooseGetContacts(DM,std::vector<std::pair<std::string,std::string> >&, std::vector<bool>&);
extern PetscErrorCode DMMooseGetUnContacts(DM,std::vector<std::pair<std::string,std::string> >&, std::vector<bool>&);
extern PetscErrorCode DMMooseSetBlocks(DM,const std::vector<std::string>&);
extern PetscErrorCode DMMooseSetVariables(DM,const std::vector<std::string>&);
extern PetscErrorCode DMMooseSetSides(DM,const std::set<std::string>&);
extern PetscErrorCode DMMooseSetUnSides(DM,const std::set<std::string>&);
extern PetscErrorCode DMMooseSetContacts(DM,const std::vector<std::pair<std::string,std::string> >&, const std::vector<bool>&);
extern PetscErrorCode DMMooseSetUnContacts(DM,const std::vector<std::pair<std::string,std::string> >&, const std::vector<bool>&);
extern PetscErrorCode DMMooseSetSplitNames(DM,const std::vector<std::string>&);
extern PetscErrorCode DMMooseGetSplitNames(DM,const std::vector<std::string>&);
extern PetscErrorCode DMMooseSetSplitVars(DM,const std::string&,const std::set<std::string>&);
extern PetscErrorCode DMMooseGetSplitVars(DM,const std::string&,std::set<std::string>&);
extern PetscErrorCode DMMooseSetSplitBlocks(DM,const std::string&,const std::set<std::string>&);
extern PetscErrorCode DMMooseGetSplitBlocks(DM,const std::string&,std::set<std::string>&);
extern PetscErrorCode DMMooseSetSplitSides(DM,const std::string&,const std::set<std::string>&);
extern PetscErrorCode DMMooseGetSplitSides(DM,const std::string&,std::set<std::string>&);


#endif // #if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
#endif // #ifdef _petscdmmoose
