//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PetscDMMoose.h"

// PETSc includes
#include <petscerror.h>
#include <petsc/private/dmimpl.h>

// MOOSE includes
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"
#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"
#include "GeometricSearchData.h"
#include "MooseVariableScalar.h"

#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/petsc_macro.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/dof_map.h"
#include "libmesh/preconditioner.h"
#include "libmesh/elem_side_builder.h"

// As of 3.18, %D is no longer supported in format strings, but the
// replacement PetscInt_FMT didn't get added until 3.7.2, and the
// libMesh shim hasn't hit our submodule yet
#if PETSC_RELEASE_LESS_THAN(3, 8, 0)
#define MOOSE_PETSCINT_FMT "D"
#else
#define MOOSE_PETSCINT_FMT PetscInt_FMT
#endif

struct DM_Moose
{
  NonlinearSystemBase * _nl;     // nonlinear system context
  std::set<std::string> * _vars; // variables
  std::map<std::string, unsigned int> * _var_ids;
  std::map<unsigned int, std::string> * _var_names;
  bool _all_vars;                  // whether all system variables are included
  std::set<std::string> * _blocks; // mesh blocks
  std::map<std::string, subdomain_id_type> * _block_ids;
  std::map<unsigned int, std::string> * _block_names;
  bool _all_blocks;               // all blocks are included
  std::set<std::string> * _sides; // mesh surfaces (edges in 2D)
  std::map<BoundaryID, std::string> * _side_names;
  std::map<std::string, BoundaryID> * _side_ids;
  std::set<std::string> * _unsides; // excluded sides
  std::map<std::string, BoundaryID> * _unside_ids;
  std::map<BoundaryID, std::string> * _unside_names;
  bool _nosides;   // whether to include any sides
  bool _nounsides; // whether to exclude any sides
  typedef std::pair<std::string, std::string> ContactName;
  typedef std::pair<BoundaryID, BoundaryID> ContactID;
  std::set<ContactName> * _contacts;
  std::map<ContactID, ContactName> * _contact_names;
  std::set<ContactName> * _uncontacts;
  std::map<ContactID, ContactName> * _uncontact_names;
  std::map<ContactName, PetscBool> * _contact_displaced;
  std::map<ContactName, PetscBool> * _uncontact_displaced;
  bool _nocontacts;
  bool _nouncontacts;
  bool _include_all_contact_nodes;
  // to locate splits without having to search, however,
  // maintain a multimap from names to split locations (to enable
  // the same split to appear in multiple spots (this might
  // break the current implementation of PCFieldSplit, though).
  std::multimap<std::string, unsigned int> * _splitlocs;
  struct SplitInfo
  {
    DM _dm;
    IS _rembedding; // relative embedding
  };
  std::map<std::string, SplitInfo> * _splits;
  IS _embedding;
  PetscBool _print_embedding;
};

#undef __FUNCT__
#define __FUNCT__ "DMMooseGetContacts"
PetscErrorCode
DMMooseGetContacts(DM dm,
                   std::vector<std::pair<std::string, std::string>> & contact_names,
                   std::vector<PetscBool> & displaced)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  DM_Moose * dmm = (DM_Moose *)dm->data;
  for (const auto & it : *(dmm->_contact_names))
  {
    contact_names.push_back(it.second);
    displaced.push_back((*dmm->_contact_displaced)[it.second]);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseGetUnContacts"
PetscErrorCode
DMMooseGetUnContacts(DM dm,
                     std::vector<std::pair<std::string, std::string>> & uncontact_names,
                     std::vector<PetscBool> & displaced)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  DM_Moose * dmm = (DM_Moose *)dm->data;
  for (const auto & it : *(dmm->_uncontact_names))
  {
    uncontact_names.push_back(it.second);
    displaced.push_back((*dmm->_uncontact_displaced)[it.second]);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseGetSides"
PetscErrorCode
DMMooseGetSides(DM dm, std::vector<std::string> & side_names)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  DM_Moose * dmm = (DM_Moose *)dm->data;
  for (const auto & it : *(dmm->_side_ids))
    side_names.push_back(it.first);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseGetUnSides"
PetscErrorCode
DMMooseGetUnSides(DM dm, std::vector<std::string> & side_names)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  DM_Moose * dmm = (DM_Moose *)dm->data;
  for (const auto & it : *(dmm->_unside_ids))
    side_names.push_back(it.first);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseGetBlocks"
PetscErrorCode
DMMooseGetBlocks(DM dm, std::vector<std::string> & block_names)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  DM_Moose * dmm = (DM_Moose *)dm->data;
  for (const auto & it : *(dmm->_block_ids))
    block_names.push_back(it.first);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseGetVariables"
PetscErrorCode
DMMooseGetVariables(DM dm, std::vector<std::string> & var_names)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  DM_Moose * dmm = (DM_Moose *)(dm->data);
  for (const auto & it : *(dmm->_var_ids))
    var_names.push_back(it.first);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseSetNonlinearSystem"
PetscErrorCode
DMMooseSetNonlinearSystem(DM dm, NonlinearSystemBase & nl)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  if (dm->setupcalled)
    SETERRQ(((PetscObject)dm)->comm,
            PETSC_ERR_ARG_WRONGSTATE,
            "Cannot reset the NonlinearSystem after DM has been set up.");
  DM_Moose * dmm = (DM_Moose *)(dm->data);
  dmm->_nl = &nl;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseSetVariables"
PetscErrorCode
DMMooseSetVariables(DM dm, const std::set<std::string> & vars)
{
  PetscErrorCode ierr;
  DM_Moose * dmm = (DM_Moose *)dm->data;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(PETSC_COMM_SELF,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  if (dm->setupcalled)
    SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "Not for an already setup DM");
  if (dmm->_vars)
    delete dmm->_vars;
  std::set<std::string> processed_vars;
  for (const auto & var_name : vars)
  {
    const auto * const var =
        dmm->_nl->hasVariable(var_name)
            ? static_cast<MooseVariableBase *>(&dmm->_nl->getVariable(0, var_name))
            : static_cast<MooseVariableBase *>(&dmm->_nl->getScalarVariable(0, var_name));
    if (var->isArray())
      for (const auto i : make_range(var->count()))
        processed_vars.insert(SubProblem::arrayVariableComponent(var_name, i));
    else
      processed_vars.insert(var_name);
  }

  dmm->_vars = new std::set<std::string>(std::move(processed_vars));
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseSetBlocks"
PetscErrorCode
DMMooseSetBlocks(DM dm, const std::set<std::string> & blocks)
{
  PetscErrorCode ierr;
  DM_Moose * dmm = (DM_Moose *)dm->data;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(PETSC_COMM_SELF,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  if (dm->setupcalled)
    SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "Not for an already setup DM");
  if (dmm->_blocks)
    delete dmm->_blocks;
  dmm->_blocks = new std::set<std::string>(blocks);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseSetSides"
PetscErrorCode
DMMooseSetSides(DM dm, const std::set<std::string> & sides)
{
  PetscErrorCode ierr;
  DM_Moose * dmm = (DM_Moose *)dm->data;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(PETSC_COMM_SELF,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  if (dm->setupcalled)
    SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "Not for an already setup DM");
  if (dmm->_sides)
    delete dmm->_sides;
  dmm->_sides = new std::set<std::string>(sides);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseSetUnSides"
PetscErrorCode
DMMooseSetUnSides(DM dm, const std::set<std::string> & unsides)
{
  PetscErrorCode ierr;
  DM_Moose * dmm = (DM_Moose *)dm->data;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(PETSC_COMM_SELF,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  if (dm->setupcalled)
    SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "Not for an already setup DM");
  if (dmm->_sides)
    delete dmm->_sides;
  dmm->_unsides = new std::set<std::string>(unsides);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseSetContacts"
PetscErrorCode
DMMooseSetContacts(DM dm,
                   const std::vector<std::pair<std::string, std::string>> & contacts,
                   const std::vector<PetscBool> & displaced)
{
  PetscErrorCode ierr;
  DM_Moose * dmm = (DM_Moose *)dm->data;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(PETSC_COMM_SELF,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  if (dm->setupcalled)
    SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "Not for an already setup DM");
  if (contacts.size() != displaced.size())
    LIBMESH_SETERRQ2(PETSC_COMM_SELF,
                     PETSC_ERR_ARG_SIZ,
                     "Nonmatching sizes of the contact and displaced arrays: %" MOOSE_PETSCINT_FMT
                     " != %" MOOSE_PETSCINT_FMT,
                     contacts.size(),
                     displaced.size());
  if (dmm->_contacts)
    delete dmm->_contacts;
  dmm->_contact_displaced->clear();
  dmm->_contacts = new std::set<DM_Moose::ContactName>();
  for (unsigned int i = 0; i < contacts.size(); ++i)
  {
    dmm->_contacts->insert(contacts[i]);
    dmm->_contact_displaced->insert(std::make_pair(contacts[i], displaced[i]));
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseSetUnContacts"
PetscErrorCode
DMMooseSetUnContacts(DM dm,
                     const std::vector<std::pair<std::string, std::string>> & uncontacts,
                     const std::vector<PetscBool> & displaced)
{
  PetscErrorCode ierr;
  DM_Moose * dmm = (DM_Moose *)dm->data;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(PETSC_COMM_SELF,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  if (dm->setupcalled)
    SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "Not for an already setup DM");
  if (uncontacts.size() != displaced.size())
    LIBMESH_SETERRQ2(PETSC_COMM_SELF,
                     PETSC_ERR_ARG_SIZ,
                     "Nonmatching sizes of the uncontact and displaced arrays: %" MOOSE_PETSCINT_FMT
                     " != %" MOOSE_PETSCINT_FMT,
                     uncontacts.size(),
                     displaced.size());
  if (dmm->_uncontacts)
    delete dmm->_uncontacts;
  dmm->_uncontact_displaced->clear();
  dmm->_uncontacts = new std::set<DM_Moose::ContactName>();
  for (unsigned int i = 0; i < uncontacts.size(); ++i)
  {
    dmm->_uncontacts->insert(uncontacts[i]);
    dmm->_uncontact_displaced->insert(std::make_pair(uncontacts[i], displaced[i]));
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseGetNonlinearSystem"
PetscErrorCode
DMMooseGetNonlinearSystem(DM dm, NonlinearSystemBase *& nl)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  DM_Moose * dmm = (DM_Moose *)(dm->data);
  nl = dmm->_nl;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseSetSplitNames"
PetscErrorCode
DMMooseSetSplitNames(DM dm, const std::vector<std::string> & split_names)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  DM_Moose * dmm = (DM_Moose *)(dm->data);

  if (dmm->_splits)
  {
    for (auto & it : *(dmm->_splits))
    {
      ierr = DMDestroy(&(it.second._dm));
      CHKERRQ(ierr);
      ierr = ISDestroy(&(it.second._rembedding));
      CHKERRQ(ierr);
    }
    delete dmm->_splits;
    dmm->_splits = PETSC_NULL;
  }
  if (dmm->_splitlocs)
  {
    delete dmm->_splitlocs;
    dmm->_splitlocs = PETSC_NULL;
  }
  dmm->_splits = new std::map<std::string, DM_Moose::SplitInfo>();
  dmm->_splitlocs = new std::multimap<std::string, unsigned int>();
  for (unsigned int i = 0; i < split_names.size(); ++i)
  {
    DM_Moose::SplitInfo info;
    info._dm = PETSC_NULL;
    info._rembedding = PETSC_NULL;
    std::string name = split_names[i];
    (*dmm->_splits)[name] = info;
    dmm->_splitlocs->insert(std::make_pair(name, i));
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseGetSplitNames"
PetscErrorCode
DMMooseGetSplitNames(DM dm, std::vector<std::string> & split_names)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "Got DM oftype %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  DM_Moose * dmm = (DM_Moose *)(dm->data);
  if (!dm->setupcalled)
    SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "DM not set up");
  split_names.clear();
  split_names.reserve(dmm->_splitlocs->size());
  if (dmm->_splitlocs && dmm->_splitlocs->size())
    for (const auto & lit : *(dmm->_splitlocs))
    {
      std::string sname = lit.first;
      unsigned int sloc = lit.second;
      split_names[sloc] = sname;
    }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseGetEmbedding_Private"
static PetscErrorCode
DMMooseGetEmbedding_Private(DM dm, IS * embedding)
{
  DM_Moose * dmm = (DM_Moose *)dm->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (!embedding)
    PetscFunctionReturn(0);
  if (!dmm->_embedding)
  {
    // The rules interpreting the coexistence of blocks (un)sides/(un)contacts are these
    // [sides and contacts behave similarly, so 'sides' means 'sides/contacts']
    // ['ANY' means 'not NONE' and covers 'ALL' as well, unless there is a specific 'ALL' clause,
    // which overrides 'ANY'; 'NOT ALL' means not ALL and not NONE]
    // [there are always some blocks, since by default 'ALL' is assumed, unless it is overridden by
    // a specific list, which implies ANY]
    // In general,
    // (1)  ALL blocks      and ANY sides are interpreted as the INTERSECTION of blocks and sides,
    // equivalent to just the sides (since ALL blocks are assumed to be a cover).
    // (2)  NOT ALL blocks  and ANY or NO sides are interpreted as the UNION of blocks and sides.
    // (3a) ANY unsides and ANY blocks are interpreted as the DIFFERENCE of blocks and unsides.
    // (3b) ANY unsides and ANY sides are interpreted as the DIFFERENCE of sides and unsides.
    // (4)  NO  unsides means NO DIFFERENCE is needed.
    // The result is easily computed by first computing the result of (1 & 2) followed by difference
    // with the result of (3 & 4).
    // To simply (1 & 2) observe the following:
    // - The intersection is computed only if ALL blocks and ANY sides, and the result is the sides,
    // so block dofs do not need to be computed.
    // - Otherwise the union is computed, and initially consists of the blocks' dofs, to which the
    // sides' dofs are added, if ANY.
    // - The result is called 'indices'
    // To satisfy (3 & 4) simply cmpute subtrahend set 'unindices' as all of the unsides' dofs:
    // Then take the set difference of 'indices' and 'unindices', putting the result in 'dindices'.
    if (!dmm->_all_vars || !dmm->_all_blocks || !dmm->_nosides || !dmm->_nounsides ||
        !dmm->_nocontacts || !dmm->_nouncontacts)
    {
      DofMap & dofmap = dmm->_nl->system().get_dof_map();
      // Put this outside the lambda scope to avoid constant memory reallocation
      std::vector<dof_id_type> node_indices;
      auto process_nodal_dof_indices =
          [&dofmap, &node_indices](const Node & node,
                                   const unsigned int var_num,
                                   std::set<dof_id_type> & local_indices,
                                   std::set<dof_id_type> * const nonlocal_indices = nullptr)
      {
        dofmap.dof_indices(&node, node_indices, var_num);
        for (const auto index : node_indices)
        {
          if (index >= dofmap.first_dof() && index < dofmap.end_dof())
            local_indices.insert(index);
          else if (nonlocal_indices)
            nonlocal_indices->insert(index);
        }
      };

      auto process_elem_dof_indices =
          [&dofmap](const std::vector<dof_id_type> & elem_indices,
                    std::set<dof_id_type> & local_indices,
                    std::set<dof_id_type> * const nonlocal_indices = nullptr)
      {
        for (const auto index : elem_indices)
        {
          if (index >= dofmap.first_dof() && index < dofmap.end_dof())
            local_indices.insert(index);
          else if (nonlocal_indices)
            nonlocal_indices->insert(index);
        }
      };

      std::set<dof_id_type> indices;
      std::set<dof_id_type> unindices;
      std::set<dof_id_type> cached_indices;
      std::set<dof_id_type> cached_unindices;
      auto & lm_mesh = dmm->_nl->system().get_mesh();
      const auto & node_to_elem_map = dmm->_nl->_fe_problem.mesh().nodeToElemMap();
      for (const auto & vit : *(dmm->_var_ids))
      {
        unsigned int v = vit.second;
        // Iterate only over this DM's blocks.
        if (!dmm->_all_blocks || (dmm->_nosides && dmm->_nocontacts))
          for (const auto & bit : *(dmm->_block_ids))
          {
            subdomain_id_type b = bit.second;
            for (const auto & elem : as_range(lm_mesh.active_local_subdomain_elements_begin(b),
                                              lm_mesh.active_local_subdomain_elements_end(b)))
            {
              // Get the degree of freedom indices for the given variable off the current element.
              std::vector<dof_id_type> evindices;
              dofmap.dof_indices(elem, evindices, v);
              process_elem_dof_indices(evindices, indices);
            }

            // Sometime, we own nodes but do not own the elements the nodes connected to
            {
              bool is_on_current_block = false;
              for (auto & node : lm_mesh.local_node_ptr_range())
              {
                const unsigned int n_comp = node->n_comp(dmm->_nl->system().number(), v);

                // skip it if no dof
                if (!n_comp)
                  continue;

                auto node_to_elem_pair = node_to_elem_map.find(node->id());
                is_on_current_block = false;
                for (const auto & elem_num : node_to_elem_pair->second)
                {
                  // if one of incident elements belongs to a block, we consider
                  // the node lives in the block
                  Elem & neighbor_elem = lm_mesh.elem_ref(elem_num);
                  if (neighbor_elem.subdomain_id() == b)
                  {
                    is_on_current_block = true;
                    break;
                  }
                }
                // we add indices for the current block only
                if (!is_on_current_block)
                  continue;

                process_nodal_dof_indices(*node, v, indices);
              }
            }
          }

        // Iterate over the sides from this split.
        if (dmm->_side_ids->size())
        {
          // For some reason the following may return an empty node list
          // std::vector<dof_id_type> snodes;
          // std::vector<boundary_id_type> sides;
          // dmm->nl->system().get_mesh().get_boundary_info().build_node_list(snodes, sides);
          // // FIXME: make an array of (snode,side) pairs, sort on side and use std::lower_bound
          // from <algorithm>
          // for (dof_id_type i = 0; i < sides.size(); ++i) {
          //   boundary_id_type s = sides[i];
          //   if (!dmm->sidenames->count(s)) continue;
          //  const Node& node = dmm->nl->system().get_mesh().node_ref(snodes[i]);
          //  // determine v's dof on node and insert into indices
          // }
          ConstBndNodeRange & bnodes = *dmm->_nl->mesh().getBoundaryNodeRange();
          for (const auto & bnode : bnodes)
          {
            BoundaryID boundary_id = bnode->_bnd_id;
            if (dmm->_side_names->find(boundary_id) == dmm->_side_names->end())
              continue;

            const Node * node = bnode->_node;
            process_nodal_dof_indices(*node, v, indices);
          }
        }

        // Iterate over the sides excluded from this split.
        if (dmm->_unside_ids->size())
        {
          ConstBndNodeRange & bnodes = *dmm->_nl->mesh().getBoundaryNodeRange();
          for (const auto & bnode : bnodes)
          {
            BoundaryID boundary_id = bnode->_bnd_id;
            if (dmm->_unside_names->find(boundary_id) == dmm->_unside_names->end())
              continue;
            const Node * node = bnode->_node;
            process_nodal_dof_indices(*node, v, unindices);
          }
        }

        auto process_contact_all_nodes =
            [dmm, process_nodal_dof_indices, v](const auto & contact_names,
                                                auto & indices_to_insert_to)
        {
          std::set<boundary_id_type> bc_id_set;
          // loop over contacts
          for (const auto & [contact_bid_pair, contact_bname_pair] : contact_names)
          {
            libmesh_ignore(contact_bname_pair);
            bc_id_set.insert(contact_bid_pair.first);  // primary
            bc_id_set.insert(contact_bid_pair.second); // secondary
          }
          // loop over boundary elements
          ConstBndElemRange & range = *dmm->_nl->_fe_problem.mesh().getBoundaryElementRange();
          for (const auto & belem : range)
          {
            const Elem * elem_bdry = belem->_elem;
            const auto side = belem->_side;
            BoundaryID boundary_id = belem->_bnd_id;

            if (bc_id_set.find(boundary_id) == bc_id_set.end())
              continue;

            for (const auto node_idx : elem_bdry->node_index_range())
              if (elem_bdry->is_node_on_side(node_idx, side))
                process_nodal_dof_indices(elem_bdry->node_ref(node_idx), v, indices_to_insert_to);
          }
        };

        auto process_contact_some_nodes =
            [dmm, process_nodal_dof_indices, v, &dofmap, &lm_mesh, process_elem_dof_indices](
                const auto & contact_names,
                auto & indices_to_insert_to,
                auto & nonlocal_indices_to_insert_to)
        {
          std::vector<dof_id_type> evindices;
          for (const auto & it : contact_names)
          {
            PetscBool displaced = (*dmm->_uncontact_displaced)[it.second];
            PenetrationLocator * locator;
            if (displaced)
            {
              std::shared_ptr<DisplacedProblem> displaced_problem =
                  dmm->_nl->_fe_problem.getDisplacedProblem();
              if (!displaced_problem)
              {
                std::ostringstream err;
                err << "Cannot use a displaced uncontact (" << it.second.first << ","
                    << it.second.second << ") with an undisplaced problem";
                mooseError(err.str());
              }
              locator = displaced_problem->geomSearchData()._penetration_locators[it.first];
            }
            else
              locator = dmm->_nl->_fe_problem.geomSearchData()._penetration_locators[it.first];

            evindices.clear();
            // penetration locator
            auto lend = locator->_penetration_info.end();
            for (auto lit = locator->_penetration_info.begin(); lit != lend; ++lit)
            {
              const dof_id_type secondary_node_num = lit->first;
              PenetrationInfo * pinfo = lit->second;
              if (pinfo && pinfo->isCaptured())
              {
                Node & secondary_node = lm_mesh.node_ref(secondary_node_num);
                process_nodal_dof_indices(
                    secondary_node, v, indices_to_insert_to, &nonlocal_indices_to_insert_to);

                // indices for primary element
                evindices.clear();
                const Elem * primary_side = pinfo->_side;
                dofmap.dof_indices(primary_side, evindices, v);
                process_elem_dof_indices(
                    evindices, indices_to_insert_to, &nonlocal_indices_to_insert_to);
              } // if pinfo
            }   // for penetration
          }     // for contact name
        };

        // Include all nodes on the contact surfaces
        if (dmm->_contact_names->size() && dmm->_include_all_contact_nodes)
          process_contact_all_nodes(*dmm->_contact_names, indices);

        // Iterate over the contacts included in this split.
        if (dmm->_contact_names->size() && !(dmm->_include_all_contact_nodes))
          process_contact_some_nodes(*dmm->_contact_names, indices, cached_indices);

        // Exclude all nodes on the contact surfaces
        if (dmm->_uncontact_names->size() && dmm->_include_all_contact_nodes)
          process_contact_all_nodes(*dmm->_uncontact_names, unindices);

        // Iterate over the contacts excluded from this split.
        if (dmm->_uncontact_names->size() && !(dmm->_include_all_contact_nodes))
          process_contact_some_nodes(*dmm->_uncontact_names, unindices, cached_unindices);
      } // variables

      std::vector<dof_id_type> local_vec_indices(cached_indices.size());
      std::copy(cached_indices.begin(), cached_indices.end(), local_vec_indices.begin());
      if (dmm->_contact_names->size() && !(dmm->_include_all_contact_nodes))
        dmm->_nl->_fe_problem.mesh().comm().allgather(local_vec_indices, false);
      // insert indices
      for (const auto & dof : local_vec_indices)
        if (dof >= dofmap.first_dof() && dof < dofmap.end_dof())
          indices.insert(dof);

      local_vec_indices.clear();
      local_vec_indices.resize(cached_unindices.size());
      std::copy(cached_unindices.begin(), cached_unindices.end(), local_vec_indices.begin());
      if (dmm->_uncontact_names->size() && !(dmm->_include_all_contact_nodes))
        dmm->_nl->_fe_problem.mesh().comm().allgather(local_vec_indices, false);
      // insert unindices
      for (const auto & dof : local_vec_indices)
        if (dof >= dofmap.first_dof() && dof < dofmap.end_dof())
          unindices.insert(dof);

      std::set<dof_id_type> dindices;
      std::set_difference(indices.begin(),
                          indices.end(),
                          unindices.begin(),
                          unindices.end(),
                          std::inserter(dindices, dindices.end()));
      PetscInt * darray;
      ierr = PetscMalloc(sizeof(PetscInt) * dindices.size(), &darray);
      CHKERRQ(ierr);
      dof_id_type i = 0;
      for (const auto & dof : dindices)
      {
        darray[i] = dof;
        ++i;
      }
      ierr = ISCreateGeneral(
          ((PetscObject)dm)->comm, dindices.size(), darray, PETSC_OWN_POINTER, &dmm->_embedding);
      CHKERRQ(ierr);
    }
    else
    {
      // if (dmm->allblocks && dmm->allvars && dmm->nosides && dmm->nounsides && dmm->nocontacts &&
      // dmm->nouncontacts)
      // DMCreateGlobalVector is defined()
      Vec v;
      PetscInt low, high;

      ierr = DMCreateGlobalVector(dm, &v);
      CHKERRQ(ierr);
      ierr = VecGetOwnershipRange(v, &low, &high);
      CHKERRQ(ierr);
      ierr = ISCreateStride(((PetscObject)dm)->comm, (high - low), low, 1, &dmm->_embedding);
      CHKERRQ(ierr);
    }
  }
  ierr = PetscObjectReference((PetscObject)(dmm->_embedding));
  CHKERRQ(ierr);
  *embedding = dmm->_embedding;

  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMCreateFieldDecomposition_Moose"
static PetscErrorCode
DMCreateFieldDecomposition_Moose(
    DM dm, PetscInt * len, char *** namelist, IS ** islist, DM ** dmlist)
{
  PetscErrorCode ierr;
  DM_Moose * dmm = (DM_Moose *)(dm->data);

  PetscFunctionBegin;

  PetscInt split_size_sum = 0;

  /* Only called after DMSetUp(). */
  if (!dmm->_splitlocs)
    PetscFunctionReturn(0);
  *len = dmm->_splitlocs->size();
  if (namelist)
  {
    ierr = PetscMalloc(*len * sizeof(char *), namelist);
    CHKERRQ(ierr);
  }
  if (islist)
  {
    ierr = PetscMalloc(*len * sizeof(IS), islist);
    CHKERRQ(ierr);
  }
  if (dmlist)
  {
    ierr = PetscMalloc(*len * sizeof(DM), dmlist);
    CHKERRQ(ierr);
  }
  for (const auto & dit : *(dmm->_splitlocs))
  {
    unsigned int d = dit.second;
    std::string dname = dit.first;
    DM_Moose::SplitInfo & dinfo = (*dmm->_splits)[dname];
    if (!dinfo._dm)
    {
      ierr = DMCreateMoose(((PetscObject)dm)->comm, *dmm->_nl, &dinfo._dm);
      CHKERRQ(ierr);
      ierr = PetscObjectSetOptionsPrefix((PetscObject)dinfo._dm, ((PetscObject)dm)->prefix);
      CHKERRQ(ierr);
      std::string suffix = std::string("fieldsplit_") + dname + "_";
      ierr = PetscObjectAppendOptionsPrefix((PetscObject)dinfo._dm, suffix.c_str());
      CHKERRQ(ierr);
    }
    ierr = DMSetFromOptions(dinfo._dm);
    CHKERRQ(ierr);
    ierr = DMSetUp(dinfo._dm);
    CHKERRQ(ierr);
    if (namelist)
    {
      ierr = PetscStrallocpy(dname.c_str(), (*namelist) + d);
      CHKERRQ(ierr);
    }
    if (islist)
    {
      if (!dinfo._rembedding)
      {
        IS dembedding, lembedding;
        ierr = DMMooseGetEmbedding_Private(dinfo._dm, &dembedding);
        CHKERRQ(ierr);
        if (dmm->_embedding)
        {
          // Create a relative embedding into the parent's index space.
          ierr = ISEmbed(dembedding, dmm->_embedding, PETSC_TRUE, &lembedding);
          CHKERRQ(ierr);
          const PetscInt * lindices;
          PetscInt len, dlen, llen, *rindices, off, i;
          ierr = ISGetLocalSize(dembedding, &dlen);
          CHKERRQ(ierr);
          ierr = ISGetLocalSize(lembedding, &llen);
          CHKERRQ(ierr);
          if (llen != dlen)
            LIBMESH_SETERRQ1(
                ((PetscObject)dm)->comm, PETSC_ERR_PLIB, "Failed to embed split %u", d);
          ierr = ISDestroy(&dembedding);
          CHKERRQ(ierr);
          // Convert local embedding to global (but still relative) embedding
          ierr = PetscMalloc(llen * sizeof(PetscInt), &rindices);
          CHKERRQ(ierr);
          ierr = ISGetIndices(lembedding, &lindices);
          CHKERRQ(ierr);
          ierr = PetscMemcpy(rindices, lindices, llen * sizeof(PetscInt));
          CHKERRQ(ierr);
          ierr = ISDestroy(&lembedding);
          CHKERRQ(ierr);
          // We could get the index offset from a corresponding global vector, but subDMs don't yet
          // have global vectors
          ierr = ISGetLocalSize(dmm->_embedding, &len);
          CHKERRQ(ierr);

          ierr = MPI_Scan(&len,
                          &off,
                          1,
#ifdef PETSC_USE_64BIT_INDICES
                          MPI_LONG_LONG_INT,
#else
                          MPI_INT,
#endif
                          MPI_SUM,
                          ((PetscObject)dm)->comm);
          CHKERRQ(ierr);

          off -= len;
          for (i = 0; i < llen; ++i)
            rindices[i] += off;
          ierr = ISCreateGeneral(
              ((PetscObject)dm)->comm, llen, rindices, PETSC_OWN_POINTER, &(dinfo._rembedding));
          CHKERRQ(ierr);
        }
        else
        {
          dinfo._rembedding = dembedding;
        }
      }
      ierr = PetscObjectReference((PetscObject)(dinfo._rembedding));
      CHKERRQ(ierr);
      (*islist)[d] = dinfo._rembedding;
      PetscInt is_size;
      ISGetLocalSize(dinfo._rembedding, &is_size);
      split_size_sum += is_size;
    }
    if (dmlist)
    {
      ierr = PetscObjectReference((PetscObject)dinfo._dm);
      CHKERRQ(ierr);
      (*dmlist)[d] = dinfo._dm;
    }
  }

  if (islist && libMesh::cast_int<libMesh::numeric_index_type>(split_size_sum) !=
                    dmm->_nl->nonlinearSolver()->system().get_system_matrix().local_m())
    mooseError("Local split size sum ",
               libMesh::cast_int<libMesh::numeric_index_type>(split_size_sum),
               " and local system matrix size ",
               dmm->_nl->nonlinearSolver()->system().get_system_matrix().local_m(),
               " do not match. Did you forget a variable or block in one of your splits?");

  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMCreateDomainDecomposition_Moose"
static PetscErrorCode
DMCreateDomainDecomposition_Moose(
    DM dm, PetscInt * len, char *** namelist, IS ** innerislist, IS ** outerislist, DM ** dmlist)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  /* Use DMCreateFieldDecomposition_Moose() to obtain everything but outerislist, which is currently
   * PETSC_NULL. */
  if (outerislist)
    *outerislist = PETSC_NULL; /* FIX: allow mesh-based overlap. */
  ierr = DMCreateFieldDecomposition_Moose(dm, len, namelist, innerislist, dmlist);
  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseFunction"
static PetscErrorCode
DMMooseFunction(DM dm, Vec x, Vec r)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  libmesh_assert(x);
  libmesh_assert(r);

  NonlinearSystemBase * nl = NULL;
  ierr = DMMooseGetNonlinearSystem(dm, nl);
  CHKERRQ(ierr);
  PetscVector<Number> & X_sys = *cast_ptr<PetscVector<Number> *>(nl->system().solution.get());
  PetscVector<Number> X_global(x, nl->comm()), R(r, nl->comm());

  // Use the system's update() to get a good local version of the
  // parallel solution.  system.update() does change the residual vector,
  // so there's no reason to swap PETSc's residual into the system for
  // this step.
  X_global.swap(X_sys);
  nl->system().update();
  X_global.swap(X_sys);

  // Enforce constraints (if any) exactly on the
  // current_local_solution.  This is the solution vector that is
  // actually used in the computation of the residual below, and is
  // not locked by debug-enabled PETSc the way that "x" is.
  nl->system().get_dof_map().enforce_constraints_exactly(nl->system(),
                                                         nl->system().current_local_solution.get());

  // Zero the residual vector before assembling
  R.zero();

  // if the user has provided both function pointers and objects only the pointer
  // will be used, so catch that as an error
  if (nl->nonlinearSolver()->residual && nl->nonlinearSolver()->residual_object)
  {
    std::ostringstream err;
    err << "ERROR: cannot specifiy both a function and object to compute the Residual!"
        << std::endl;
    mooseError(err.str());
  }
  if (nl->nonlinearSolver()->matvec && nl->nonlinearSolver()->residual_and_jacobian_object)
  {
    std::ostringstream err;
    err << "ERROR: cannot specifiy both a function and object to compute the combined Residual & "
           "Jacobian!"
        << std::endl;
    mooseError(err.str());
  }
  if (nl->nonlinearSolver()->residual != NULL)
    nl->nonlinearSolver()->residual(
        *(nl->system().current_local_solution.get()), R, nl->nonlinearSolver()->system());
  else if (nl->nonlinearSolver()->residual_object != NULL)
    nl->nonlinearSolver()->residual_object->residual(
        *(nl->system().current_local_solution.get()), R, nl->nonlinearSolver()->system());
  else if (nl->nonlinearSolver()->matvec != NULL)
    nl->nonlinearSolver()->matvec(
        *(nl->system().current_local_solution.get()), &R, NULL, nl->nonlinearSolver()->system());
  else if (nl->nonlinearSolver()->residual_and_jacobian_object != NULL)
    nl->nonlinearSolver()->residual_and_jacobian_object->residual_and_jacobian(
        *(nl->system().current_local_solution.get()), &R, NULL, nl->nonlinearSolver()->system());
  else
  {
    std::ostringstream err;
    err << "No suitable residual computation routine found";
    mooseError(err.str());
  }
  R.close();
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "SNESFunction_DMMoose"
static PetscErrorCode
SNESFunction_DMMoose(SNES, Vec x, Vec r, void * ctx)
{
  DM dm = (DM)ctx;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DMMooseFunction(dm, x, r);
  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseJacobian"
static PetscErrorCode
DMMooseJacobian(DM dm, Vec x, Mat jac, Mat pc)
{
  PetscErrorCode ierr;
  NonlinearSystemBase * nl = NULL;

  PetscFunctionBegin;
  ierr = DMMooseGetNonlinearSystem(dm, nl);
  CHKERRQ(ierr);

  PetscMatrix<Number> the_pc(pc, nl->comm());
  PetscMatrix<Number> Jac(jac, nl->comm());
  PetscVector<Number> & X_sys = *cast_ptr<PetscVector<Number> *>(nl->system().solution.get());
  PetscVector<Number> X_global(x, nl->comm());

  // Set the dof maps
  the_pc.attach_dof_map(nl->system().get_dof_map());
  Jac.attach_dof_map(nl->system().get_dof_map());

  // Use the system's update() to get a good local version of the
  // parallel solution.  system.update() does change the Jacobian, so
  // there's no reason to swap PETSc's Jacobian into the system for
  // this step.
  X_global.swap(X_sys);
  nl->system().update();
  X_global.swap(X_sys);

  // Enforce constraints (if any) exactly on the
  // current_local_solution.  This is the solution vector that is
  // actually used in the computation of the Jacobian below, and is
  // not locked by debug-enabled PETSc the way that "x" is.
  nl->system().get_dof_map().enforce_constraints_exactly(nl->system(),
                                                         nl->system().current_local_solution.get());

  // Zero out the preconditioner before computing the Jacobian.
  the_pc.zero();

  // if the user has provided both function pointers and objects only the pointer
  // will be used, so catch that as an error
  if (nl->nonlinearSolver()->jacobian && nl->nonlinearSolver()->jacobian_object)
  {
    std::ostringstream err;
    err << "ERROR: cannot specifiy both a function and object to compute the Jacobian!"
        << std::endl;
    mooseError(err.str());
  }
  if (nl->nonlinearSolver()->matvec && nl->nonlinearSolver()->residual_and_jacobian_object)
  {
    std::ostringstream err;
    err << "ERROR: cannot specifiy both a function and object to compute the combined Residual & "
           "Jacobian!"
        << std::endl;
    mooseError(err.str());
  }
  if (nl->nonlinearSolver()->jacobian != NULL)
    nl->nonlinearSolver()->jacobian(
        *(nl->system().current_local_solution.get()), the_pc, nl->nonlinearSolver()->system());
  else if (nl->nonlinearSolver()->jacobian_object != NULL)
    nl->nonlinearSolver()->jacobian_object->jacobian(
        *(nl->system().current_local_solution.get()), the_pc, nl->nonlinearSolver()->system());
  else if (nl->nonlinearSolver()->matvec != NULL)
    nl->nonlinearSolver()->matvec(*(nl->system().current_local_solution.get()),
                                  NULL,
                                  &the_pc,
                                  nl->nonlinearSolver()->system());
  else if (nl->nonlinearSolver()->residual_and_jacobian_object != NULL)
    nl->nonlinearSolver()->residual_and_jacobian_object->residual_and_jacobian(
        *(nl->system().current_local_solution.get()),
        NULL,
        &the_pc,
        nl->nonlinearSolver()->system());
  else
  {
    std::ostringstream err;
    err << "No suitable Jacobian routine or object";
    mooseError(err.str());
  }
  the_pc.close();
  Jac.close();
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "SNESJacobian_DMMoose"
static PetscErrorCode
SNESJacobian_DMMoose(SNES, Vec x, Mat jac, Mat pc, void * ctx)
{
  DM dm = (DM)ctx;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DMMooseJacobian(dm, x, jac, pc);
  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMVariableBounds_Moose"
static PetscErrorCode
DMVariableBounds_Moose(DM dm, Vec xl, Vec xu)
{
  PetscErrorCode ierr;
  NonlinearSystemBase * nl = NULL;

  PetscFunctionBegin;
  ierr = DMMooseGetNonlinearSystem(dm, nl);
  CHKERRQ(ierr);

  PetscVector<Number> XL(xl, nl->comm());
  PetscVector<Number> XU(xu, nl->comm());

  ierr = VecSet(xl, PETSC_NINFINITY);
  CHKERRQ(ierr);
  ierr = VecSet(xu, PETSC_INFINITY);
  CHKERRQ(ierr);
  if (nl->nonlinearSolver()->bounds != NULL)
    nl->nonlinearSolver()->bounds(XL, XU, nl->nonlinearSolver()->system());
  else if (nl->nonlinearSolver()->bounds_object != NULL)
    nl->nonlinearSolver()->bounds_object->bounds(XL, XU, nl->nonlinearSolver()->system());
  else
    SETERRQ(
        ((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "No bounds calculation in this Moose object");
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMCreateGlobalVector_Moose"
static PetscErrorCode
DMCreateGlobalVector_Moose(DM dm, Vec * x)
{
  PetscErrorCode ierr;
  DM_Moose * dmm = (DM_Moose *)(dm->data);
  PetscBool ismoose;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "DM of type %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  if (!dmm->_nl)
    SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");

  NumericVector<Number> * nv = (dmm->_nl->system().solution).get();
  PetscVector<Number> * pv = dynamic_cast<PetscVector<Number> *>(nv);
  Vec v = pv->vec();
  /* Unfortunately, currently this does not produce a ghosted vector, so nonlinear subproblem solves
   aren't going to be easily available.
   Should work fine for getting vectors out for linear subproblem solvers. */
  if (dmm->_embedding)
  {
    PetscInt n;
    ierr = VecCreate(((PetscObject)v)->comm, x);
    CHKERRQ(ierr);
    ierr = ISGetLocalSize(dmm->_embedding, &n);
    CHKERRQ(ierr);
    ierr = VecSetSizes(*x, n, PETSC_DETERMINE);
    CHKERRQ(ierr);
    ierr = VecSetType(*x, ((PetscObject)v)->type_name);
    CHKERRQ(ierr);
    ierr = VecSetFromOptions(*x);
    CHKERRQ(ierr);
    ierr = VecSetUp(*x);
    CHKERRQ(ierr);
  }
  else
  {
    ierr = VecDuplicate(v, x);
    CHKERRQ(ierr);
  }

#if PETSC_RELEASE_LESS_THAN(3, 13, 0)
  ierr = PetscObjectCompose((PetscObject)*x, "DM", (PetscObject)dm);
  CHKERRQ(ierr);
#else
  ierr = VecSetDM(*x, dm);
  CHKERRQ(ierr);
#endif
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMCreateMatrix_Moose"
static PetscErrorCode
DMCreateMatrix_Moose(DM dm, Mat * A)
{
  PetscErrorCode ierr;
  DM_Moose * dmm = (DM_Moose *)(dm->data);
  PetscBool ismoose;
  MatType type;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "DM of type %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  if (!dmm->_nl)
    SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");
  ierr = DMGetMatType(dm, &type);
  CHKERRQ(ierr);

  /*
   The simplest thing for now: compute the sparsity_pattern using dof_map and init the matrix using
   that info.
   TODO: compute sparsity restricted to this DM's blocks, variables and sides.
   Even fancier: compute the sparsity of the coupling of a contact secondary to the contact primary.
   In any event, here we are in control of the matrix type and structure.
   */
  DofMap & dof_map = dmm->_nl->system().get_dof_map();
  PetscInt M, N, m, n;
  MPI_Comm comm;
  M = dof_map.n_dofs();
  N = M;
  m = static_cast<PetscInt>(dof_map.n_dofs_on_processor(dmm->_nl->system().processor_id()));
  n = m;
  ierr = PetscObjectGetComm((PetscObject)dm, &comm);
  CHKERRQ(ierr);
  ierr = MatCreate(comm, A);
  CHKERRQ(ierr);
  ierr = MatSetSizes(*A, m, n, M, N);
  CHKERRQ(ierr);
  ierr = MatSetType(*A, type);
  CHKERRQ(ierr);
  /* Set preallocation for the basic sparse matrix types (applies only if *A has the right type. */
  /* For now we ignore blocksize issues, since BAIJ doesn't play well with field decomposition by
   * variable. */
  const std::vector<numeric_index_type> & n_nz = dof_map.get_n_nz();
  const std::vector<numeric_index_type> & n_oz = dof_map.get_n_oz();
  ierr = MatSeqAIJSetPreallocation(*A, 0, (PetscInt *)(n_nz.empty() ? NULL : &n_nz[0]));
  CHKERRQ(ierr);
  ierr = MatMPIAIJSetPreallocation(*A,
                                   0,
                                   (PetscInt *)(n_nz.empty() ? NULL : &n_nz[0]),
                                   0,
                                   (PetscInt *)(n_oz.empty() ? NULL : &n_oz[0]));
  CHKERRQ(ierr);
  /* TODO: set the prefix for *A and MatSetFromOptions(*A)? Might override the type and other
   * settings made here. */
  ierr = MatSetUp(*A);
  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMView_Moose"
static PetscErrorCode
DMView_Moose(DM dm, PetscViewer viewer)
{
  PetscErrorCode ierr;
  PetscBool isascii;
  const char *name, *prefix;
  DM_Moose * dmm = (DM_Moose *)dm->data;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)viewer, PETSCVIEWERASCII, &isascii);
  CHKERRQ(ierr);
  if (isascii)
  {
    ierr = PetscObjectGetName((PetscObject)dm, &name);
    CHKERRQ(ierr);
    ierr = PetscObjectGetOptionsPrefix((PetscObject)dm, &prefix);
    CHKERRQ(ierr);
    ierr = PetscViewerASCIIPrintf(viewer, "DM Moose with name %s and prefix %s\n", name, prefix);
    CHKERRQ(ierr);
    ierr = PetscViewerASCIIPrintf(viewer, "variables:");
    CHKERRQ(ierr);
    for (const auto & vit : *(dmm->_var_ids))
    {
      ierr = PetscViewerASCIIPrintf(viewer, "(%s,%u) ", vit.first.c_str(), vit.second);
      CHKERRQ(ierr);
    }
    ierr = PetscViewerASCIIPrintf(viewer, "\n");
    CHKERRQ(ierr);
    ierr = PetscViewerASCIIPrintf(viewer, "blocks:");
    CHKERRQ(ierr);
    for (const auto & bit : *(dmm->_block_ids))
    {
      ierr = PetscViewerASCIIPrintf(viewer, "(%s,%d) ", bit.first.c_str(), bit.second);
      CHKERRQ(ierr);
    }
    ierr = PetscViewerASCIIPrintf(viewer, "\n");
    CHKERRQ(ierr);

    if (dmm->_side_ids->size())
    {
      ierr = PetscViewerASCIIPrintf(viewer, "sides:");
      CHKERRQ(ierr);
      for (const auto & sit : *(dmm->_side_ids))
      {
        ierr = PetscViewerASCIIPrintf(viewer, "(%s,%d) ", sit.first.c_str(), sit.second);
        CHKERRQ(ierr);
      }
      ierr = PetscViewerASCIIPrintf(viewer, "\n");
      CHKERRQ(ierr);
    }

    if (dmm->_unside_ids->size())
    {
      ierr = PetscViewerASCIIPrintf(viewer, "unsides:");
      CHKERRQ(ierr);
      for (const auto & sit : *(dmm->_unside_ids))
      {
        ierr = PetscViewerASCIIPrintf(viewer, "(%s,%d) ", sit.first.c_str(), sit.second);
        CHKERRQ(ierr);
      }
      ierr = PetscViewerASCIIPrintf(viewer, "\n");
      CHKERRQ(ierr);
    }

    if (dmm->_contact_names->size())
    {
      ierr = PetscViewerASCIIPrintf(viewer, "contacts:");
      CHKERRQ(ierr);
      for (const auto & cit : *(dmm->_contact_names))
      {
        ierr = PetscViewerASCIIPrintf(
            viewer, "(%s,%s,", cit.second.first.c_str(), cit.second.second.c_str());
        CHKERRQ(ierr);
        if ((*dmm->_contact_displaced)[cit.second])
        {
          ierr = PetscViewerASCIIPrintf(viewer, "displaced) ");
          CHKERRQ(ierr);
        }
        else
        {
          ierr = PetscViewerASCIIPrintf(viewer, "undisplaced) ");
          CHKERRQ(ierr);
        }
      }
      ierr = PetscViewerASCIIPrintf(viewer, "\n");
      CHKERRQ(ierr);
    }

    if (dmm->_uncontact_names->size())
    {
      ierr = PetscViewerASCIIPrintf(viewer, "_uncontacts:");
      CHKERRQ(ierr);
      for (const auto & cit : *(dmm->_uncontact_names))
      {
        ierr = PetscViewerASCIIPrintf(
            viewer, "(%s,%s,", cit.second.first.c_str(), cit.second.second.c_str());
        CHKERRQ(ierr);
        if ((*dmm->_uncontact_displaced)[cit.second])
        {
          ierr = PetscViewerASCIIPrintf(viewer, "displaced) ");
          CHKERRQ(ierr);
        }
        else
        {
          ierr = PetscViewerASCIIPrintf(viewer, "undisplaced) ");
          CHKERRQ(ierr);
        }
      }
      ierr = PetscViewerASCIIPrintf(viewer, "\n");
      CHKERRQ(ierr);
    }

    if (dmm->_splitlocs && dmm->_splitlocs->size())
    {
      ierr = PetscViewerASCIIPrintf(viewer, "Field decomposition:");
      CHKERRQ(ierr);
      // FIX: decompositions might have different sizes and components on different ranks.
      for (const auto & dit : *(dmm->_splitlocs))
      {
        std::string dname = dit.first;
        ierr = PetscViewerASCIIPrintf(viewer, " %s", dname.c_str());
        CHKERRQ(ierr);
      }
      ierr = PetscViewerASCIIPrintf(viewer, "\n");
      CHKERRQ(ierr);
    }
  }
  else
    SETERRQ(PETSC_COMM_SELF, PETSC_ERR_SUP, "Non-ASCII viewers are not supported");

  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseGetMeshBlocks_Private"
static PetscErrorCode
DMMooseGetMeshBlocks_Private(DM dm, std::set<subdomain_id_type> & blocks)
{
  PetscErrorCode ierr;
  DM_Moose * dmm = (DM_Moose *)(dm->data);
  PetscBool ismoose;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "DM of type %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  if (!dmm->_nl)
    SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");

  const MeshBase & mesh = dmm->_nl->system().get_mesh();
  /* The following effectively is a verbatim copy of MeshBase::n_subdomains(). */
  // This requires an inspection on every processor
  libmesh_parallel_only(mesh.comm());
  for (const auto & elem : mesh.active_element_ptr_range())
    blocks.insert(elem->subdomain_id());
  // Some subdomains may only live on other processors
  mesh.comm().set_union(blocks);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMSetUp_Moose_Pre"
static PetscErrorCode
DMSetUp_Moose_Pre(DM dm)
{
  PetscErrorCode ierr;
  DM_Moose * dmm = (DM_Moose *)(dm->data);
  PetscBool ismoose;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "DM of type %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  if (!dmm->_nl)
    SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");

  /* Set up variables, blocks and sides. */
  DofMap & dofmap = dmm->_nl->system().get_dof_map();
  /* libMesh mesh */
  const MeshBase & mesh = dmm->_nl->system().get_mesh();

  dmm->_nosides = PETSC_TRUE;
  dmm->_side_ids->clear();
  dmm->_side_names->clear();
  if (dmm->_sides)
  {
    dmm->_nosides = PETSC_FALSE;
    std::set<BoundaryID> ids;
    for (const auto & name : *(dmm->_sides))
    {
      boundary_id_type id = dmm->_nl->mesh().getBoundaryID(name);
      dmm->_side_names->insert(std::make_pair(id, name));
      dmm->_side_ids->insert(std::make_pair(name, id));
    }
    delete dmm->_sides;
    dmm->_sides = PETSC_NULL;
  }
  dmm->_nounsides = PETSC_TRUE;
  dmm->_unside_ids->clear();
  dmm->_unside_names->clear();
  if (dmm->_unsides)
  {
    dmm->_nounsides = PETSC_FALSE;
    std::set<BoundaryID> ids;
    for (const auto & name : *(dmm->_unsides))
    {
      boundary_id_type id = dmm->_nl->mesh().getBoundaryID(name);
      dmm->_unside_names->insert(std::make_pair(id, name));
      dmm->_unside_ids->insert(std::make_pair(name, id));
    }
    delete dmm->_unsides;
    dmm->_unsides = PETSC_NULL;
  }
  dmm->_nocontacts = PETSC_TRUE;

  if (dmm->_contacts)
  {
    dmm->_nocontacts = PETSC_FALSE;
    for (const auto & cpair : *(dmm->_contacts))
    {
      try
      {
        if ((*dmm->_contact_displaced)[cpair])
          dmm->_nl->_fe_problem.getDisplacedProblem()->geomSearchData().getPenetrationLocator(
              cpair.first, cpair.second);
        else
          dmm->_nl->_fe_problem.geomSearchData().getPenetrationLocator(cpair.first, cpair.second);
      }
      catch (...)
      {
        std::ostringstream err;
        err << "Problem retrieving contact for PenetrationLocator with primary " << cpair.first
            << " and secondary " << cpair.second;
        mooseError(err.str());
      }
      BoundaryID primary_id = dmm->_nl->mesh().getBoundaryID(cpair.first);
      BoundaryID secondary_id = dmm->_nl->mesh().getBoundaryID(cpair.second);
      DM_Moose::ContactID cid(primary_id, secondary_id);
      dmm->_contact_names->insert(std::make_pair(cid, cpair));
    }
  }

  dmm->_nouncontacts = PETSC_TRUE;
  if (dmm->_uncontacts)
  {
    dmm->_nouncontacts = PETSC_FALSE;
    for (const auto & cpair : *(dmm->_uncontacts))
    {
      try
      {
        if ((*dmm->_uncontact_displaced)[cpair])
          dmm->_nl->_fe_problem.getDisplacedProblem()->geomSearchData().getPenetrationLocator(
              cpair.first, cpair.second);
        else
          dmm->_nl->_fe_problem.geomSearchData().getPenetrationLocator(cpair.first, cpair.second);
      }
      catch (...)
      {
        std::ostringstream err;
        err << "Problem retrieving uncontact for PenetrationLocator with primary " << cpair.first
            << " and secondary " << cpair.second;
        mooseError(err.str());
      }
      BoundaryID primary_id = dmm->_nl->mesh().getBoundaryID(cpair.first);
      BoundaryID secondary_id = dmm->_nl->mesh().getBoundaryID(cpair.second);
      DM_Moose::ContactID cid(primary_id, secondary_id);
      dmm->_uncontact_names->insert(std::make_pair(cid, cpair));
    }
  }

  dmm->_var_ids->clear();
  dmm->_var_names->clear();
  // FIXME: would be nice to invert this nested loop structure so we could iterate over the
  // potentially smaller dmm->vars,
  // but checking against dofmap.variable would still require a linear search, hence, no win.  Would
  // be nice to endow dofmap.variable
  // with a fast search capability.
  for (unsigned int v = 0; v < dofmap.n_variables(); ++v)
  {
    std::string vname = dofmap.variable(v).name();
    if (dmm->_vars && dmm->_vars->size() && dmm->_vars->find(vname) == dmm->_vars->end())
      continue;
    dmm->_var_ids->insert(std::pair<std::string, unsigned int>(vname, v));
    dmm->_var_names->insert(std::pair<unsigned int, std::string>(v, vname));
  }
  if (dmm->_var_ids->size() == dofmap.n_variables())
    dmm->_all_vars = PETSC_TRUE;
  else
    dmm->_all_vars = PETSC_FALSE;
  if (dmm->_vars)
  {
    delete dmm->_vars;
    dmm->_vars = PETSC_NULL;
  }

  dmm->_block_ids->clear();
  dmm->_block_names->clear();
  std::set<subdomain_id_type> blocks;
  ierr = DMMooseGetMeshBlocks_Private(dm, blocks);
  CHKERRQ(ierr);
  if (blocks.empty())
    SETERRQ(((PetscObject)dm)->comm, PETSC_ERR_PLIB, "No mesh blocks found.");

  for (const auto & bid : blocks)
  {
    std::string bname = mesh.subdomain_name(bid);
    if (!bname.length())
    {
      // Block names are currently implemented for Exodus II meshes
      // only, so we might have to make up our own block names and
      // maintain our own mapping of block ids to names.
      std::ostringstream ss;
      ss << bid;
      bname = ss.str();
    }
    if (dmm->_nosides && dmm->_nocontacts)
    {
      // If no sides and no contacts have been specified, by default (null or empty dmm->blocks) all
      // blocks are included in the split Thus, skip this block only if it is explicitly excluded
      // from a nonempty dmm->blocks.
      if (dmm->_blocks && dmm->_blocks->size() &&
          (dmm->_blocks->find(bname) ==
               dmm->_blocks->end() && // We should allow users to use subdomain IDs
           dmm->_blocks->find(std::to_string(bid)) == dmm->_blocks->end()))
        continue;
    }
    else
    {
      // If sides or contacts have been specified, only the explicitly-specified blocks (those in
      // dmm->blocks, if it's non-null) are in the split. Thus, include this block only if it is
      // explicitly specified in a nonempty dmm->blocks. Equivalently, skip this block if
      // dmm->blocks is dmm->blocks is null or empty or excludes this block.
      if (!dmm->_blocks || !dmm->_blocks->size() ||
          (dmm->_blocks->find(bname) ==
               dmm->_blocks->end() // We should allow users to use subdomain IDs
           && dmm->_blocks->find(std::to_string(bid)) == dmm->_blocks->end()))
        continue;
    }
    dmm->_block_ids->insert(std::make_pair(bname, bid));
    dmm->_block_names->insert(std::make_pair(bid, bname));
  }

  if (dmm->_block_ids->size() == blocks.size())
    dmm->_all_blocks = PETSC_TRUE;
  else
    dmm->_all_blocks = PETSC_FALSE;
  if (dmm->_blocks)
  {
    delete dmm->_blocks;
    dmm->_blocks = PETSC_NULL;
  }

  std::string name = dmm->_nl->system().name();
  name += "_vars";
  for (const auto & vit : *(dmm->_var_names))
    name += "_" + vit.second;

  name += "_blocks";

  for (const auto & bit : *(dmm->_block_names))
    name += "_" + bit.second;

  if (dmm->_side_names && dmm->_side_names->size())
  {
    name += "_sides";
    for (const auto & sit : *(dmm->_side_names))
      name += "_" + sit.second;
  }
  if (dmm->_unside_names && dmm->_unside_names->size())
  {
    name += "_unsides";
    for (const auto & sit : *(dmm->_unside_names))
      name += "_" + sit.second;
  }
  if (dmm->_contact_names && dmm->_contact_names->size())
  {
    name += "_contacts";
    for (const auto & cit : *(dmm->_contact_names))
      name += "_primary_" + cit.second.first + "_secondary_" + cit.second.second;
  }
  if (dmm->_uncontact_names && dmm->_uncontact_names->size())
  {
    name += "_uncontacts";
    for (const auto & cit : *(dmm->_uncontact_names))
      name += "_primary_" + cit.second.first + "_secondary_" + cit.second.second;
  }
  ierr = PetscObjectSetName((PetscObject)dm, name.c_str());
  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseReset"
PetscErrorCode
DMMooseReset(DM dm)
{
  PetscErrorCode ierr;
  DM_Moose * dmm = (DM_Moose *)(dm->data);
  PetscBool ismoose;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    PetscFunctionReturn(0);
  if (!dmm->_nl)
    SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");
  ierr = ISDestroy(&dmm->_embedding);
  CHKERRQ(ierr);
  for (auto & it : *(dmm->_splits))
  {
    DM_Moose::SplitInfo & split = it.second;
    ierr = ISDestroy(&split._rembedding);
    CHKERRQ(ierr);
    if (split._dm)
    {
      ierr = DMMooseReset(split._dm);
      CHKERRQ(ierr);
    }
  }
  dm->setupcalled = PETSC_FALSE;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMSetUp_Moose"
static PetscErrorCode
DMSetUp_Moose(DM dm)
{
  PetscErrorCode ierr;
  DM_Moose * dmm = (DM_Moose *)(dm->data);
  PetscBool ismoose;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "DM of type %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  if (!dmm->_nl)
    SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");
  if (dmm->_print_embedding)
  {
    const char *name, *prefix;
    IS embedding;

    ierr = PetscObjectGetName((PetscObject)dm, &name);
    CHKERRQ(ierr);
    ierr = PetscObjectGetOptionsPrefix((PetscObject)dm, &prefix);
    CHKERRQ(ierr);
    ierr = PetscViewerASCIIPrintf(PETSC_VIEWER_STDOUT_(((PetscObject)dm)->comm),
                                  "DM Moose with name %s and prefix %s\n",
                                  name,
                                  prefix);
    CHKERRQ(ierr);
    if (dmm->_all_vars && dmm->_all_blocks && dmm->_nosides && dmm->_nounsides &&
        dmm->_nocontacts && dmm->_nouncontacts)
    {
      ierr = PetscViewerASCIIPrintf(PETSC_VIEWER_STDOUT_(((PetscObject)dm)->comm),
                                    "\thas a trivial embedding\n");
      CHKERRQ(ierr);
    }
    else
    {
      ierr = DMMooseGetEmbedding_Private(dm, &embedding);
      CHKERRQ(ierr);
      ierr = PetscViewerASCIIPrintf(PETSC_VIEWER_STDOUT_(((PetscObject)dm)->comm),
                                    "\thas embedding defined by IS:\n");
      CHKERRQ(ierr);
      ierr = ISView(embedding, PETSC_VIEWER_STDOUT_(((PetscObject)dm)->comm));
      CHKERRQ(ierr);
      ierr = ISDestroy(&embedding);
      CHKERRQ(ierr);
    }
  }
  /*
   Do not evaluate function, Jacobian or bounds for an embedded DM -- the subproblem might not have
   enough information for that.
   */
  if (dmm->_all_vars && dmm->_all_blocks && dmm->_nosides && dmm->_nounsides && dmm->_nocontacts &&
      dmm->_nouncontacts)
  {
    ierr = DMSNESSetFunction(dm, SNESFunction_DMMoose, (void *)dm);
    CHKERRQ(ierr);
    ierr = DMSNESSetJacobian(dm, SNESJacobian_DMMoose, (void *)dm);
    CHKERRQ(ierr);
    if (dmm->_nl->nonlinearSolver()->bounds || dmm->_nl->nonlinearSolver()->bounds_object)
      ierr = DMSetVariableBounds(dm, DMVariableBounds_Moose);
    CHKERRQ(ierr);
  }
  else
  {
    /*
     Fow now we don't implement even these, although a linear "Dirichlet" subproblem is
     well-defined.
     Creating the submatrix, however, might require extracting the submatrix preallocation from an
     unassembled matrix.
     */
    dm->ops->createglobalvector = 0;
    dm->ops->creatematrix = 0;
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMSetFromOptions_Moose"
#if !PETSC_VERSION_LESS_THAN(3, 18, 0)
PetscErrorCode
DMSetFromOptions_Moose(DM dm, PetscOptionItems * /*options*/) // >= 3.18.0
#elif !PETSC_VERSION_LESS_THAN(3, 7, 0)
PetscErrorCode
DMSetFromOptions_Moose(PetscOptionItems * /*options*/, DM dm) // >= 3.7.0
#else
PetscErrorCode
DMSetFromOptions_Moose(PetscOptions * /*options*/, DM dm) // >= 3.6.0
#endif
{
  PetscErrorCode ierr;
  PetscBool ismoose;
  DM_Moose * dmm = (DM_Moose *)dm->data;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
  CHKERRQ(ierr);
  if (!ismoose)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_WRONG,
                     "DM of type %s, not of type %s",
                     ((PetscObject)dm)->type_name,
                     DMMOOSE);
  if (!dmm->_nl)
    SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");
// PETSc changed macro definitions in 3.18; the former correct usage
// is now a compiler error and the new usage is now a compiler
// warning.
#if !PETSC_VERSION_LESS_THAN(3, 18, 0)
  PetscOptionsBegin(((PetscObject)dm)->comm, ((PetscObject)dm)->prefix, "DMMoose options", "DM");
#else
  ierr = PetscOptionsBegin(
      ((PetscObject)dm)->comm, ((PetscObject)dm)->prefix, "DMMoose options", "DM");
#endif
  std::string opt, help;
  PetscInt maxvars = dmm->_nl->system().get_dof_map().n_variables();
  char ** vars;
  std::set<std::string> varset;
  PetscInt nvars = maxvars;
  ierr = PetscMalloc(maxvars * sizeof(char *), &vars);
  CHKERRQ(ierr);
  opt = "-dm_moose_vars";
  help = "Variables in DMMoose";
  ierr = PetscOptionsStringArray(
      opt.c_str(), help.c_str(), "DMMooseSetVars", vars, &nvars, PETSC_NULL);
  CHKERRQ(ierr);
  for (PetscInt i = 0; i < nvars; ++i)
  {
    varset.insert(std::string(vars[i]));
    ierr = PetscFree(vars[i]);
    CHKERRQ(ierr);
  }
  ierr = PetscFree(vars);
  CHKERRQ(ierr);
  if (varset.size())
  {
    ierr = DMMooseSetVariables(dm, varset);
    CHKERRQ(ierr);
  }
  //
  std::set<subdomain_id_type> meshblocks;
  ierr = DMMooseGetMeshBlocks_Private(dm, meshblocks);
  CHKERRQ(ierr);
  PetscInt maxblocks = meshblocks.size();
  char ** blocks;
  ierr = PetscMalloc(maxblocks * sizeof(char *), &blocks);
  CHKERRQ(ierr);
  std::set<std::string> blockset;
  PetscInt nblocks = maxblocks;
  opt = "-dm_moose_blocks";
  help = "Blocks in DMMoose";
  ierr = PetscOptionsStringArray(
      opt.c_str(), help.c_str(), "DMMooseSetBlocks", blocks, &nblocks, PETSC_NULL);
  CHKERRQ(ierr);
  for (PetscInt i = 0; i < nblocks; ++i)
  {
    blockset.insert(std::string(blocks[i]));
    ierr = PetscFree(blocks[i]);
    CHKERRQ(ierr);
  }
  ierr = PetscFree(blocks);
  CHKERRQ(ierr);
  if (blockset.size())
  {
    ierr = DMMooseSetBlocks(dm, blockset);
    CHKERRQ(ierr);
  }
  PetscInt maxsides = dmm->_nl->system().get_mesh().get_boundary_info().get_boundary_ids().size();
  char ** sides;
  ierr = PetscMalloc(maxsides * sizeof(char *), &sides);
  CHKERRQ(ierr);
  PetscInt nsides = maxsides;
  std::set<std::string> sideset;
  opt = "-dm_moose_sides";
  help = "Sides to include in DMMoose";
  ierr = PetscOptionsStringArray(
      opt.c_str(), help.c_str(), "DMMooseSetSides", sides, &nsides, PETSC_NULL);
  CHKERRQ(ierr);
  for (PetscInt i = 0; i < nsides; ++i)
  {
    sideset.insert(std::string(sides[i]));
    ierr = PetscFree(sides[i]);
    CHKERRQ(ierr);
  }
  if (sideset.size())
  {
    ierr = DMMooseSetSides(dm, sideset);
    CHKERRQ(ierr);
  }
  opt = "-dm_moose_unsides";
  help = "Sides to exclude from DMMoose";
  nsides = maxsides;
  ierr = PetscOptionsStringArray(
      opt.c_str(), help.c_str(), "DMMooseSetUnSides", sides, &nsides, PETSC_NULL);
  CHKERRQ(ierr);
  sideset.clear();
  for (PetscInt i = 0; i < nsides; ++i)
  {
    sideset.insert(std::string(sides[i]));
    ierr = PetscFree(sides[i]);
    CHKERRQ(ierr);
  }
  if (sideset.size())
  {
    ierr = DMMooseSetUnSides(dm, sideset);
    CHKERRQ(ierr);
  }
  ierr = PetscFree(sides);
  CHKERRQ(ierr);
  PetscInt maxcontacts = dmm->_nl->_fe_problem.geomSearchData()._penetration_locators.size();
  std::shared_ptr<DisplacedProblem> displaced_problem = dmm->_nl->_fe_problem.getDisplacedProblem();
  if (displaced_problem)
    maxcontacts = PetscMax(
        maxcontacts, (PetscInt)displaced_problem->geomSearchData()._penetration_locators.size());

  std::vector<DM_Moose::ContactName> contacts;
  std::vector<PetscBool> contact_displaced;
  PetscInt ncontacts = 0;
  opt = "-dm_moose_ncontacts";
  help =
      "Number of contacts to include in DMMoose.  For each <n> < "
      "dm_moose_contacts\n\t-dm_moose_contact_<n> is a comma-separated <primary>,<secondary> pair "
      "defining the contact surfaces"
      "\t-dm_moose_contact_<n>_displaced <bool> determines whether the contact is defined on "
      "the displaced mesh or not";
  ierr = PetscOptionsInt(
      opt.c_str(), help.c_str(), "DMMooseSetContacts", ncontacts, &ncontacts, PETSC_NULL);
  CHKERRQ(ierr);
  if (ncontacts > maxcontacts)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_SIZ,
                     "Number of requested contacts %" MOOSE_PETSCINT_FMT
                     " exceeds the maximum number of contacts %" MOOSE_PETSCINT_FMT,
                     ncontacts,
                     maxcontacts);
  for (PetscInt i = 0; i < ncontacts; ++i)
  {
    {
      char * primary_secondary[2];
      PetscInt sz = 2;
      std::ostringstream oopt, ohelp;
      oopt << "-dm_moose_contact_" << i;
      ohelp << "Primary and secondary for contact " << i;
      ierr = PetscOptionsStringArray(oopt.str().c_str(),
                                     ohelp.str().c_str(),
                                     "DMMooseSetContacts",
                                     primary_secondary,
                                     &sz,
                                     PETSC_NULL);
      CHKERRQ(ierr);
      if (sz != 2)
        LIBMESH_SETERRQ2(
            ((PetscObject)dm)->comm,
            PETSC_ERR_ARG_SIZ,
            "Expected 2 sideset IDs (primary & secondary) for contact %" MOOSE_PETSCINT_FMT
            ", got %" MOOSE_PETSCINT_FMT " instead",
            i,
            sz);
      contacts.push_back(DM_Moose::ContactName(std::string(primary_secondary[0]),
                                               std::string(primary_secondary[1])));
      ierr = PetscFree(primary_secondary[0]);
      CHKERRQ(ierr);
      ierr = PetscFree(primary_secondary[1]);
      CHKERRQ(ierr);
    }
    {
      PetscBool displaced = PETSC_FALSE;
      std::ostringstream oopt, ohelp;
      oopt << "-dm_moose_contact_" << i << "_displaced";
      ohelp << "Whether contact " << i << " is determined using displaced mesh or not";
      ierr = PetscOptionsBool(oopt.str().c_str(),
                              ohelp.str().c_str(),
                              "DMMooseSetContacts",
                              PETSC_FALSE,
                              &displaced,
                              PETSC_NULL);
      CHKERRQ(ierr);
      contact_displaced.push_back(displaced);
    }
  }
  if (contacts.size())
  {
    ierr = DMMooseSetContacts(dm, contacts, contact_displaced);
    CHKERRQ(ierr);
  }
  {
    std::ostringstream oopt, ohelp;
    PetscBool is_include_all_nodes;
    oopt << "-dm_moose_includeAllContactNodes";
    ohelp << "Whether to include all nodes on the contact surfaces into the subsolver";
    ierr = PetscOptionsBool(oopt.str().c_str(),
                            ohelp.str().c_str(),
                            "",
                            PETSC_FALSE,
                            &is_include_all_nodes,
                            PETSC_NULL);
    CHKERRQ(ierr);
    dmm->_include_all_contact_nodes = is_include_all_nodes;
  }
  std::vector<DM_Moose::ContactName> uncontacts;
  std::vector<PetscBool> uncontact_displaced;
  PetscInt nuncontacts = 0;
  opt = "-dm_moose_nuncontacts";
  help =
      "Number of contacts to exclude from DMMoose.  For each <n> < "
      "dm_moose_contacts\n\t-dm_moose_contact_<n> is a comma-separated <primary>,<secondary> pair "
      "defining the contact surfaces"
      "\t-dm_moose_contact_<n>_displaced <bool> determines whether the contact is defined on "
      "the displaced mesh or not";
  ierr = PetscOptionsInt(
      opt.c_str(), help.c_str(), "DMMooseSetUnContacts", nuncontacts, &nuncontacts, PETSC_NULL);
  CHKERRQ(ierr);
  if (nuncontacts > maxcontacts)
    LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                     PETSC_ERR_ARG_SIZ,
                     "Number of requested uncontacts %" MOOSE_PETSCINT_FMT
                     " exceeds the maximum number of contacts %" MOOSE_PETSCINT_FMT,
                     nuncontacts,
                     maxcontacts);
  for (PetscInt i = 0; i < nuncontacts; ++i)
  {
    {
      char * primary_secondary[2];
      PetscInt sz = 2;
      std::ostringstream oopt, ohelp;
      oopt << "-dm_moose_uncontact_" << i;
      ohelp << "Primary and secondary for uncontact " << i;
      ierr = PetscOptionsStringArray(oopt.str().c_str(),
                                     ohelp.str().c_str(),
                                     "DMMooseSetUnContacts",
                                     primary_secondary,
                                     &sz,
                                     PETSC_NULL);
      CHKERRQ(ierr);
      if (sz != 2)
        LIBMESH_SETERRQ2(
            ((PetscObject)dm)->comm,
            PETSC_ERR_ARG_SIZ,
            "Expected 2 sideset IDs (primary & secondary) for uncontact %" MOOSE_PETSCINT_FMT
            ", got %" MOOSE_PETSCINT_FMT " instead",
            i,
            sz);
      uncontacts.push_back(DM_Moose::ContactName(std::string(primary_secondary[0]),
                                                 std::string(primary_secondary[1])));
      ierr = PetscFree(primary_secondary[0]);
      CHKERRQ(ierr);
      ierr = PetscFree(primary_secondary[1]);
      CHKERRQ(ierr);
    }
    {
      PetscBool displaced = PETSC_FALSE;
      std::ostringstream oopt, ohelp;
      oopt << "-dm_moose_uncontact_" << i << "_displaced";
      ohelp << "Whether uncontact " << i << " is determined using displaced mesh or not";
      ierr = PetscOptionsBool(oopt.str().c_str(),
                              ohelp.str().c_str(),
                              "DMMooseSetUnContact",
                              PETSC_FALSE,
                              &displaced,
                              PETSC_NULL);
      CHKERRQ(ierr);
      uncontact_displaced.push_back(displaced);
    }
  }
  if (uncontacts.size())
  {
    ierr = DMMooseSetUnContacts(dm, uncontacts, uncontact_displaced);
    CHKERRQ(ierr);
  }

  PetscInt nsplits = 0;
  /* Insert the usage of -dm_moose_fieldsplit_names into this help message, since the following
   * if-clause might never fire, if -help is requested. */
  const char * fdhelp = "Number of named fieldsplits defined by the DM.\n\
                \tNames of fieldsplits are defined by -dm_moose_fieldsplit_names <splitname1> <splitname2> ...\n\
                \tEach split can be configured with its own variables, blocks and sides, as any DMMoose";
  ierr = PetscOptionsInt(
      "-dm_moose_nfieldsplits", fdhelp, "DMMooseSetSplitNames", nsplits, &nsplits, NULL);
  CHKERRQ(ierr);
  if (nsplits)
  {
    PetscInt nnsplits = nsplits;
    std::vector<std::string> split_names;
    char ** splitnames;
    ierr = PetscMalloc(nsplits * sizeof(char *), &splitnames);
    CHKERRQ(ierr);
    ierr = PetscOptionsStringArray("-dm_moose_fieldsplit_names",
                                   "Names of fieldsplits defined by the DM",
                                   "DMMooseSetSplitNames",
                                   splitnames,
                                   &nnsplits,
                                   PETSC_NULL);
    CHKERRQ(ierr);
    if (!nnsplits)
    {
      for (PetscInt i = 0; i < nsplits; ++i)
      {
        std::ostringstream s;
        s << i;
        split_names.push_back(s.str());
      }
    }
    else if (nsplits != nnsplits)
      LIBMESH_SETERRQ2(((PetscObject)dm)->comm,
                       PETSC_ERR_ARG_SIZ,
                       "Expected %" MOOSE_PETSCINT_FMT " fieldsplit names, got %" MOOSE_PETSCINT_FMT
                       " instead",
                       nsplits,
                       nnsplits);
    else
    {
      for (PetscInt i = 0; i < nsplits; ++i)
      {
        split_names.push_back(std::string(splitnames[i]));
        ierr = PetscFree(splitnames[i]);
        CHKERRQ(ierr);
      }
    }
    ierr = PetscFree(splitnames);
    CHKERRQ(ierr);
    ierr = DMMooseSetSplitNames(dm, split_names);
    CHKERRQ(ierr);
  }
  ierr = PetscOptionsBool("-dm_moose_print_embedding",
                          "Print IS embedding DM's dofs",
                          "DMMoose",
                          dmm->_print_embedding,
                          &dmm->_print_embedding,
                          PETSC_NULL);
  CHKERRQ(ierr);
  /**
   * Unused value warning for GCC was introduced for the PetscOptionsEnd() macro in PETSc 3.17.0
   * via PETSc MR !4889. Fixed up in PETSc 3.18.0 via PETSc MR !5069, so these pragmas can be
   * removed when we update to that release.
   */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
  PetscOptionsEnd();
#pragma GCC diagnostic pop
  ierr = DMSetUp_Moose_Pre(dm);
  CHKERRQ(ierr); /* Need some preliminary set up because, strangely enough, DMView() is called in
                    DMSetFromOptions(). */
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMDestroy_Moose"
static PetscErrorCode
DMDestroy_Moose(DM dm)
{
  DM_Moose * dmm = (DM_Moose *)(dm->data);
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (dmm->_vars)
    delete dmm->_vars;
  delete dmm->_var_ids;
  delete dmm->_var_names;
  if (dmm->_blocks)
    delete dmm->_blocks;
  delete dmm->_block_ids;
  delete dmm->_block_names;
  if (dmm->_sides)
    delete dmm->_sides;
  delete dmm->_side_ids;
  delete dmm->_side_names;
  if (dmm->_unsides)
    delete dmm->_unsides;
  delete dmm->_unside_ids;
  delete dmm->_unside_names;
  if (dmm->_contacts)
    delete dmm->_contacts;
  delete dmm->_contact_names;
  delete dmm->_contact_displaced;
  if (dmm->_uncontacts)
    delete dmm->_uncontacts;
  delete dmm->_uncontact_names;
  delete dmm->_uncontact_displaced;
  if (dmm->_splits)
  {
    for (auto & sit : *(dmm->_splits))
    {
      ierr = DMDestroy(&(sit.second._dm));
      CHKERRQ(ierr);
      ierr = ISDestroy(&(sit.second._rembedding));
      CHKERRQ(ierr);
    }
    delete dmm->_splits;
  }
  if (dmm->_splitlocs)
    delete dmm->_splitlocs;
  ierr = ISDestroy(&dmm->_embedding);
  CHKERRQ(ierr);
  ierr = PetscFree(dm->data);
  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMCreateMoose"
PetscErrorCode
DMCreateMoose(MPI_Comm comm, NonlinearSystemBase & nl, DM * dm)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DMCreate(comm, dm);
  CHKERRQ(ierr);
  ierr = DMSetType(*dm, DMMOOSE);
  CHKERRQ(ierr);
  ierr = DMMooseSetNonlinearSystem(*dm, nl);
  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

EXTERN_C_BEGIN
#undef __FUNCT__
#define __FUNCT__ "DMCreate_Moose"
PetscErrorCode
DMCreate_Moose(DM dm)
{
  PetscErrorCode ierr;
  DM_Moose * dmm;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
#if PETSC_RELEASE_LESS_THAN(3, 18, 0)
  ierr = PetscNewLog(dm, &dmm);
  CHKERRQ(ierr);
#else // PetscNewLog was deprecated
  ierr = PetscNew(&dmm);
  CHKERRQ(ierr);
#endif
  dm->data = dmm;

  dmm->_var_ids = new (std::map<std::string, unsigned int>);
  dmm->_block_ids = new (std::map<std::string, subdomain_id_type>);
  dmm->_var_names = new (std::map<unsigned int, std::string>);
  dmm->_block_names = new (std::map<unsigned int, std::string>);
  dmm->_side_ids = new (std::map<std::string, BoundaryID>);
  dmm->_side_names = new (std::map<BoundaryID, std::string>);
  dmm->_unside_ids = new (std::map<std::string, BoundaryID>);
  dmm->_unside_names = new (std::map<BoundaryID, std::string>);
  dmm->_contact_names = new (std::map<DM_Moose::ContactID, DM_Moose::ContactName>);
  dmm->_uncontact_names = new (std::map<DM_Moose::ContactID, DM_Moose::ContactName>);
  dmm->_contact_displaced = new (std::map<DM_Moose::ContactName, PetscBool>);
  dmm->_uncontact_displaced = new (std::map<DM_Moose::ContactName, PetscBool>);

  dmm->_splits = new (std::map<std::string, DM_Moose::SplitInfo>);

  dmm->_print_embedding = PETSC_FALSE;

  dm->ops->createglobalvector = DMCreateGlobalVector_Moose;
  dm->ops->createlocalvector = 0; // DMCreateLocalVector_Moose;
  dm->ops->getcoloring = 0;       // DMGetColoring_Moose;
  dm->ops->creatematrix = DMCreateMatrix_Moose;
  dm->ops->createinterpolation = 0; // DMCreateInterpolation_Moose;

  dm->ops->refine = 0;  // DMRefine_Moose;
  dm->ops->coarsen = 0; // DMCoarsen_Moose;
#if PETSC_RELEASE_LESS_THAN(3, 12, 0)
  dm->ops->getinjection = 0;  // DMGetInjection_Moose;
  dm->ops->getaggregates = 0; // DMGetAggregates_Moose;
#else
  dm->ops->createinjection = 0;
#endif

  dm->ops->createfielddecomposition = DMCreateFieldDecomposition_Moose;
  dm->ops->createdomaindecomposition = DMCreateDomainDecomposition_Moose;

  dm->ops->destroy = DMDestroy_Moose;
  dm->ops->view = DMView_Moose;
  dm->ops->setfromoptions = DMSetFromOptions_Moose;
  dm->ops->setup = DMSetUp_Moose;
  PetscFunctionReturn(0);
}
EXTERN_C_END

#undef __FUNCT__
#define __FUNCT__ "SNESUpdateDMMoose"
PetscErrorCode
SNESUpdateDMMoose(SNES snes, PetscInt iteration)
{
  /* This is called any time the structure of the problem changes in a way that affects the Jacobian
     sparsity pattern.
     For example, this may happen when NodeFaceConstraints change Jacobian's sparsity pattern based
     on newly-detected Penetration.
     In that case certain preconditioners (e.g., PCASM) will not work, unless we tell them that the
     sparsity pattern has changed.
     For now we are rebuilding the whole KSP, when necessary.
  */
  PetscErrorCode ierr;
  DM dm;
  KSP ksp;
  const char * prefix;
  MPI_Comm comm;
  PC pc;

  PetscFunctionBegin;
  if (iteration)
  {
    /* TODO: limit this only to situations when displaced (un)contact splits are present, as is
     * DisplacedProblem(). */
    ierr = SNESGetDM(snes, &dm);
    CHKERRQ(ierr);
    ierr = DMMooseReset(dm);
    CHKERRQ(ierr);
    ierr = DMSetUp(dm);
    CHKERRQ(ierr);
    ierr = SNESGetKSP(snes, &ksp);
    CHKERRQ(ierr);
    /* Should we rebuild the whole KSP? */
    ierr = PetscObjectGetOptionsPrefix((PetscObject)ksp, &prefix);
    CHKERRQ(ierr);
    ierr = PetscObjectGetComm((PetscObject)ksp, &comm);
    CHKERRQ(ierr);
    ierr = PCCreate(comm, &pc);
    CHKERRQ(ierr);
    ierr = PCSetDM(pc, dm);
    CHKERRQ(ierr);
    ierr = PCSetOptionsPrefix(pc, prefix);
    CHKERRQ(ierr);
    ierr = PCSetFromOptions(pc);
    CHKERRQ(ierr);
    ierr = KSPSetPC(ksp, pc);
    CHKERRQ(ierr);
    ierr = PCDestroy(&pc);
    CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseRegisterAll"
PetscErrorCode
DMMooseRegisterAll()
{
  static PetscBool DMMooseRegisterAllCalled = PETSC_FALSE;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (!DMMooseRegisterAllCalled)
  {
    ierr = DMRegister(DMMOOSE, DMCreate_Moose);
    CHKERRQ(ierr);
    DMMooseRegisterAllCalled = PETSC_TRUE;
  }
  PetscFunctionReturn(0);
}
