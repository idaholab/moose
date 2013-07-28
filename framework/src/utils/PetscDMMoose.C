#include "libmesh/petsc_macro.h"
// This only works with petsc-3.3 and above.

#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)

#include "PetscDMMoose.h"

// PETSc includes
#include <petscerror.h>
#include <petsc-private/dmimpl.h>

// libMesh Includes
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/petsc_dm_nonlinear_solver.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/dof_map.h"
#include "libmesh/preconditioner.h"

#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"
#include "GeometricSearchData.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"

struct DM_Moose
{
  NonlinearSystem*                         nl;
  std::set<std::string>                    *vars;
  std::map<std::string, unsigned int>      *varids;
  std::map<unsigned int, std::string>      *varnames;
  bool                                     allvars; // whether all system variables are included
  std::set<std::string>                    *blocks;
  std::map<std::string, unsigned int>      *blockids;
  std::map<unsigned int, std::string>      *blocknames;
  bool                                     allblocks;
  std::set<std::string>                    *sides;
  std::set<std::string>                    *unsides;
  std::map<BoundaryID, std::string>        *sidenames;
  std::map<std::string, BoundaryID>        *sideids;
  std::map<std::string, BoundaryID>        *unsideids;
  std::map<BoundaryID, std::string>        *unsidenames;
  bool                                     nosides;   // whether to include any sides
  bool                                     nounsides; // whether to exclude any sides
  typedef std::pair<std::string,std::string> ContactName;
  typedef std::pair<BoundaryID,BoundaryID>   ContactID;
  std::set<ContactName>                    *contacts;
  std::map<ContactID,ContactName>          *contactnames;
  std::set<ContactName>                    *uncontacts;
  std::map<ContactID,ContactName>          *uncontactnames;
  std::map<ContactName,PetscBool>          *contact_displaced;
  std::map<ContactName,PetscBool>          *uncontact_displaced;
  bool                                     nocontacts;
  bool                                     nouncontacts;
  // to locate splits without having to search, however,
  // maintain a multimap from names to split locations (to enable
  // the same split to appear in multiple spots (this might
  // break the current implementation of PCFieldSplit, though).
  std::multimap<std::string, unsigned int> *splitlocs;
  struct SplitInfo {
    DM  dm;
    IS  rembedding; // relative embedding
  };
  std::map<std::string, SplitInfo >        *splits;
  IS                                       embedding;
  PetscBool                                print_embedding;
};

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetContacts"
PetscErrorCode DMMooseGetContacts(DM dm, std::vector<std::pair<std::string,std::string> >& contactnames, std::vector<PetscBool>& displaced)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)dm->data;
  for (std::map<DM_Moose::ContactID,DM_Moose::ContactName>::const_iterator it = dmm->contactnames->begin(); it != dmm->contactnames->end(); ++it) {
    contactnames.push_back(it->second);
    displaced.push_back((*dmm->contact_displaced)[it->second]);
  }
  PetscFunctionReturn(0);
 }

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetUnContacts"
PetscErrorCode DMMooseGetUnContacts(DM dm, std::vector<std::pair<std::string,std::string> >& uncontactnames, std::vector<PetscBool>& displaced)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)dm->data;
  for (std::map<DM_Moose::ContactID,DM_Moose::ContactName>::const_iterator it = dmm->uncontactnames->begin(); it != dmm->uncontactnames->end(); ++it) {
    uncontactnames.push_back(it->second);
    displaced.push_back((*dmm->uncontact_displaced)[it->second]);
  }
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetSides"
PetscErrorCode DMMooseGetSides(DM dm, std::vector<std::string>& sidenames)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)dm->data;
  for (std::map<std::string, BoundaryID>::const_iterator it = dmm->sideids->begin(); it != dmm->sideids->end(); ++it) sidenames.push_back(it->first);
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetUnSides"
PetscErrorCode DMMooseGetUnSides(DM dm, std::vector<std::string>& sidenames)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)dm->data;
  for (std::map<std::string, BoundaryID>::const_iterator it = dmm->unsideids->begin(); it != dmm->unsideids->end(); ++it) sidenames.push_back(it->first);
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetBlocks"
PetscErrorCode DMMooseGetBlocks(DM dm, std::vector<std::string>& blocknames)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)dm->data;
  for (std::map<std::string, unsigned int>::const_iterator it = dmm->blockids->begin(); it != dmm->blockids->end(); ++it) blocknames.push_back(it->first);
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetVariables"
PetscErrorCode DMMooseGetVariables(DM dm, std::vector<std::string>& varnames)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);
  for (std::map<std::string, unsigned int>::const_iterator it = dmm->varids->begin(); it != dmm->varids->end(); ++it){
    varnames.push_back(it->first);
  }
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseSetNonlinearSystem"
PetscErrorCode DMMooseSetNonlinearSystem(DM dm, NonlinearSystem& nl)
{
  PetscErrorCode ierr;
  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  PetscBool ismoose;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);

  if(dm->setupcalled) SETERRQ(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONGSTATE, "Cannot reset the NonlinearSystem after DM has been set up.");
  DM_Moose *dmm = (DM_Moose *)(dm->data);
  dmm->nl =&nl;
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseSetVariables"
PetscErrorCode DMMooseSetVariables(DM dm, const std::set<std::string>& vars)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose*)dm->data;
  PetscBool      ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose); CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  if(dm->setupcalled) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "Not for an already setup DM");
  if (dmm->vars) delete dmm->vars;
  dmm->vars = new std::set<std::string>(vars);
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseSetBlocks"
PetscErrorCode DMMooseSetBlocks(DM dm, const std::set<std::string>& blocks)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose*)dm->data;
  PetscBool      ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose); CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  if(dm->setupcalled) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "Not for an already setup DM");
  if (dmm->blocks) delete dmm->blocks;
  dmm->blocks = new std::set<std::string>(blocks);
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseSetSides"
PetscErrorCode DMMooseSetSides(DM dm, const std::set<std::string>& sides)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose*)dm->data;
  PetscBool      ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose); CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  if(dm->setupcalled) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "Not for an already setup DM");
  if (dmm->sides) delete dmm->sides;
  dmm->sides = new std::set<std::string>(sides);
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseSetUnSides"
PetscErrorCode DMMooseSetUnSides(DM dm, const std::set<std::string>& unsides)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose*)dm->data;
  PetscBool      ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose); CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  if(dm->setupcalled) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "Not for an already setup DM");
  if (dmm->sides) delete dmm->sides;
  dmm->unsides = new std::set<std::string>(unsides);
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseSetContacts"
PetscErrorCode DMMooseSetContacts(DM dm, const std::vector<std::pair<std::string,std::string> >& contacts, const std::vector<PetscBool>& displaced)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose*)dm->data;
  PetscBool      ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose); CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  if(dm->setupcalled) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "Not for an already setup DM");
  if (contacts.size() != displaced.size()) SETERRQ2(PETSC_COMM_SELF, PETSC_ERR_ARG_SIZ, "Nonmatching sizes of the contact and displaced arrays: %D != %D", contacts.size(), displaced.size());
  if (dmm->contacts) delete dmm->contacts;
  dmm->contact_displaced->clear();
  dmm->contacts = new std::set<DM_Moose::ContactName>();
  for (unsigned int i = 0; i < contacts.size(); ++i) {
    dmm->contacts->insert(contacts[i]);
    dmm->contact_displaced->insert(std::pair<DM_Moose::ContactName,PetscBool>(contacts[i],displaced[i]));
  }
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseSetUnContacts"
PetscErrorCode DMMooseSetUnContacts(DM dm, const std::vector<std::pair<std::string,std::string> >& uncontacts, const std::vector<PetscBool>& displaced)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose*)dm->data;
  PetscBool      ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose); CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  if(dm->setupcalled) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "Not for an already setup DM");
  if (uncontacts.size() != displaced.size()) SETERRQ2(PETSC_COMM_SELF, PETSC_ERR_ARG_SIZ, "Nonmatching sizes of the uncontact and displaced arrays: %D != %D", uncontacts.size(), displaced.size());
  if (dmm->uncontacts) delete dmm->uncontacts;
  dmm->uncontact_displaced->clear();
  dmm->uncontacts = new std::set<DM_Moose::ContactName>();
  for (unsigned int i = 0; i < uncontacts.size(); ++i) {
    dmm->uncontacts->insert(uncontacts[i]);
    dmm->uncontact_displaced->insert(std::pair<DM_Moose::ContactName,PetscBool>(uncontacts[i],displaced[i]));
  }
  PetscFunctionReturn(0);
}


#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetNonlinearSystem"
PetscErrorCode DMMooseGetNonlinearSystem(DM dm, NonlinearSystem*& nl)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose); CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);
  nl = dmm->nl;
  PetscFunctionReturn(0);
}


#undef  __FUNCT__
#define __FUNCT__ "DMMooseSetSplitNames"
PetscErrorCode DMMooseSetSplitNames(DM dm, const std::vector<std::string>& splitnames)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  PetscBool ismoose;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);

  if (dmm->splits) {
    for (std::map<std::string,DM_Moose::SplitInfo>::iterator it = dmm->splits->begin(); it != dmm->splits->end(); ++it) {
      ierr = DMDestroy(&(it->second.dm));CHKERRQ(ierr);
      ierr = ISDestroy(&(it->second.rembedding));CHKERRQ(ierr);
    }
    delete dmm->splits;
    dmm->splits = PETSC_NULL;
  }
  if (dmm->splitlocs) {
    delete dmm->splitlocs;
    dmm->splitlocs = PETSC_NULL;
  }
  dmm->splits   = new std::map<std::string, DM_Moose::SplitInfo>();
  dmm->splitlocs = new std::multimap<std::string,unsigned int>();
  for (unsigned int i = 0; i < splitnames.size(); ++i) {
    DM_Moose::SplitInfo info;
    info.dm = PETSC_NULL;
    info.rembedding = PETSC_NULL;
    std::string name = splitnames[i];
    (*dmm->splits)[name] = info;
    std::pair<std::string,unsigned int> pair(name,i);
    dmm->splitlocs->insert(pair);
  }
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetSplitNames"
PetscErrorCode DMMooseGetSplitNames(DM dm, std::vector<std::string>& splitnames)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  PetscBool ismoose;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);
  if (!dm->setupcalled) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "DM not set up");
  splitnames.clear();
  splitnames.reserve(dmm->splitlocs->size());
  if (dmm->splitlocs && dmm->splitlocs->size()) {
    for (std::multimap<std::string, unsigned int >::const_iterator lit = dmm->splitlocs->begin(); lit != dmm->splitlocs->end(); ++lit) {
      std::string sname = lit->first;
      unsigned int sloc = lit->second;
      splitnames[sloc] = sname;
    }
  }
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetEmbedding_Private"
static PetscErrorCode DMMooseGetEmbedding_Private(DM dm, IS *embedding)
{
  DM_Moose       *dmm=(DM_Moose*)dm->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (!embedding) PetscFunctionReturn(0);
  if (!dmm->embedding) {
    /* The rules interpreting the coexistence of blocks (un)sides/(un)contacts are these
       [sides and contacts behave similarly, so 'sides' means 'sides/contacts']
       ['ANY' means 'not NONE' and covers 'ALL' as well, unless there is a specific 'ALL' clause, which overrides 'ANY'; 'NOT ALL' means not ALL and not NONE]
       [there are always some blocks, since by default 'ALL' is assumed, unless it is overridden by a specific list, which implies ANY]
       In general,
          (1)  ALL blocks      and ANY sides are interpreted as the INTERSECTION of blocks and sides, equivalent to just the sides (since ALL blocks are assumed to be a cover).
          (2)  NOT ALL blocks  and ANY or NO sides are interpreted as the UNION of blocks and sides.
	  (3a) ANY unsides and ANY blocks are interpreted as the DIFFERENCE of blocks and unsides.
	  (3b) ANY unsides and ANY sides are interpreted as the DIFFERENCE of sides and unsides.
          (4)  NO  unsides means NO DIFFERENCE is needed.
       The result is easily computed by first computing the result of (1 & 2) followed by difference with the result of (3 & 4).
       To simply (1 & 2) observe the following:
          - The intersection is computed only if ALL blocks and ANY sides, and the result is the sides, so block dofs do not need to be computed.
	  - Otherwise the union is computed, and initially consists of the blocks' dofs, to which the sides' dofs are added, if ANY.
	  - The result is called 'indices'
       To satisfy (3 & 4) simply cmpute subtrahend set 'unindices' as all of the unsides' dofs:
       Then take the set difference of 'indices' and 'unindices', putting the result in 'dindices'.
    */

    if (!dmm->allvars || !dmm->allblocks || !dmm->nosides || !dmm->nounsides || !dmm->nocontacts || !dmm->nouncontacts) {
      DofMap& dofmap = dmm->nl->sys().get_dof_map();
      std::set<unsigned int>               indices;
      std::set<unsigned int> unindices;
      for(std::map<std::string, unsigned int>::const_iterator vit = dmm->varids->begin(); vit != dmm->varids->end(); ++vit){
	unsigned int v = vit->second;
	/* Iterate only over this DM's blocks. */
	if (!dmm->allblocks || (dmm->nosides && dmm->nocontacts)) {
	  for(std::map<std::string, unsigned int>::const_iterator bit = dmm->blockids->begin(); bit != dmm->blockids->end(); ++bit) {
	    unsigned int b = bit->second;
	    MeshBase::const_element_iterator el     = dmm->nl->sys().get_mesh().active_local_subdomain_elements_begin(b);
	    MeshBase::const_element_iterator end_el = dmm->nl->sys().get_mesh().active_local_subdomain_elements_end(b);
	    for ( ; el != end_el; ++el) {
	      const Elem* elem = *el;
	      std::vector<unsigned int> evindices;
	      // Get the degree of freedom indices for the given variable off the current element.
	      dofmap.dof_indices(elem, evindices, v);
	      for(unsigned int i = 0; i < evindices.size(); ++i) {
		unsigned int dof = evindices[i];
		if(dof >= dofmap.first_dof() && dof < dofmap.end_dof()) /* might want to use variable_first/last_local_dof instead */
		  indices.insert(dof);
	      }
	    }
	  }
	}
	/* Iterate over the sides from this split. */
	if (dmm->sideids->size()) {
	  // For some reason the following may return an empty node list
	  // std::vector<unsigned int> snodes;
	  // std::vector<boundary_id_type> sides;
	  // dmm->nl->sys().get_mesh().boundary_info->build_node_list(snodes, sides);
	  // // FIXME: make an array of (snode,side) pairs, sort on side and use std::lower_bound from <algorithm>
	  // for (unsigned int i = 0; i < sides.size(); ++i) {
	  //   boundary_id_type s = sides[i];
	  //   if (!dmm->sidenames->count(s)) continue;
	  //  const Node& node = dmm->nl->sys().get_mesh().node(snodes[i]);
	  //  // determine v's dof on node and insert into indices
	  // }
 	  ConstBndNodeRange & bnodes = *dmm->nl->mesh().getBoundaryNodeRange();
	  for (ConstBndNodeRange::const_iterator bnodeit = bnodes.begin(); bnodeit != bnodes.end(); ++bnodeit) {
	    const BndNode * bnode = *bnodeit;
	    BoundaryID      boundary_id = bnode->_bnd_id;
	    if (dmm->sidenames->find(boundary_id) == dmm->sidenames->end()) continue;
	    const Node*     node = bnode->_node;
	    dof_id_type dof = node->dof_number(dmm->nl->sys().number(),v,0);
	    if(dof >= dofmap.first_dof() && dof < dofmap.end_dof()) { /* might want to use variable_first/last_local_dof instead */
	      indices.insert(dof);
	    }
	  }
	}
	/* Iterate over the sides excluded from this split. */
	if (dmm->unsideids->size()) {
	  ConstBndNodeRange & bnodes = *dmm->nl->mesh().getBoundaryNodeRange();
	  for (ConstBndNodeRange::const_iterator bnodeit = bnodes.begin(); bnodeit != bnodes.end(); ++bnodeit) {
	    const BndNode * bnode = *bnodeit;
	    BoundaryID      boundary_id = bnode->_bnd_id;
	    if (dmm->unsidenames->find(boundary_id) == dmm->unsidenames->end()) continue;
	    const Node*     node = bnode->_node;
	    dof_id_type dof = node->dof_number(dmm->nl->sys().number(),v,0);
	    if(dof >= dofmap.first_dof() && dof < dofmap.end_dof()) { /* might want to use variable_first/last_local_dof instead */
	      unindices.insert(dof);
	    }
	  }
	}
	/* Iterate over the contacts included in this split. */
	if (dmm->contactnames->size()) {
	  for (std::map<DM_Moose::ContactID, DM_Moose::ContactName>::const_iterator it = dmm->contactnames->begin(); it != dmm->contactnames->end(); ++it) {
	    PetscBool displaced = (*dmm->contact_displaced)[it->second];
	    PenetrationLocator *locator;
	    if (displaced) {
	      DisplacedProblem *displaced_problem = dmm->nl->_fe_problem.getDisplacedProblem();
	      if (!displaced_problem) {
		std::ostringstream err;
		err << "Cannot use a displaced contact (" << it->second.first << "," << it->second.second << ") with an undisplaced problem";
		mooseError(err.str());
	      }
	      locator = displaced_problem->geomSearchData()._penetration_locators[it->first];
	    } else {
	      locator = dmm->nl->_fe_problem.geomSearchData()._penetration_locators[it->first];
	    }
	    std::vector<unsigned int>& slave_nodes = locator->_nearest_node._slave_nodes;
	    for (unsigned int i = 0; i < slave_nodes.size(); ++i) {
	      if (locator->_has_penetrated.find(slave_nodes[i]) == locator->_has_penetrated.end()) continue;
	      Node& slave_node = dmm->nl->sys().get_mesh().node(slave_nodes[i]);
	      dof_id_type dof = slave_node.dof_number(dmm->nl->sys().number(),v,0);
	      if(dof >= dofmap.first_dof() && dof < dofmap.end_dof()) { /* might want to use variable_first/last_local_dof instead */
		indices.insert(dof);
	      }
	    }
	  }
	}
	/* Iterate over the contacts excluded from this split. */
	if (dmm->uncontactnames->size()) {
	  for (std::map<DM_Moose::ContactID, DM_Moose::ContactName>::const_iterator it = dmm->uncontactnames->begin(); it != dmm->uncontactnames->end(); ++it) {
	    PetscBool displaced = (*dmm->uncontact_displaced)[it->second];
	    PenetrationLocator *locator;
	    if (displaced) {
	      DisplacedProblem *displaced_problem = dmm->nl->_fe_problem.getDisplacedProblem();
	      if (!displaced_problem) {
		std::ostringstream err;
		err << "Cannot use a displaced uncontact (" << it->second.first << "," << it->second.second << ") with an undisplaced problem";
		mooseError(err.str());
	      }
	      locator = displaced_problem->geomSearchData()._penetration_locators[it->first];
	    } else {
	      locator = dmm->nl->_fe_problem.geomSearchData()._penetration_locators[it->first];
	    }
	    std::vector<unsigned int>& slave_nodes = locator->_nearest_node._slave_nodes;
	    for (unsigned int i = 0; i < slave_nodes.size(); ++i) {
	      if (locator->_has_penetrated.find(slave_nodes[i]) == locator->_has_penetrated.end()) continue;
	      Node& slave_node = dmm->nl->sys().get_mesh().node(slave_nodes[i]);
	      dof_id_type dof = slave_node.dof_number(dmm->nl->sys().number(),v,0);
	      if(dof >= dofmap.first_dof() && dof < dofmap.end_dof()) { /* might want to use variable_first/last_local_dof instead */
		unindices.insert(dof);
	      }
	    }
	  }
	}
      }
      std::set<unsigned int> dindices;
      std::set_difference(indices.begin(),indices.end(),unindices.begin(),unindices.end(),std::inserter(dindices,dindices.end()));
      PetscInt *darray;
      ierr = PetscMalloc(sizeof(PetscInt)*dindices.size(),&darray);CHKERRQ(ierr);
      unsigned int i = 0;
      for(std::set<unsigned int>::const_iterator it = dindices.begin(); it != dindices.end(); ++it) {
	darray[i] = *it;
	++i;
      }
      ierr = ISCreateGeneral(((PetscObject)dm)->comm, dindices.size(),darray, PETSC_OWN_POINTER, &dmm->embedding); CHKERRQ(ierr);
    } else { /* if (dmm->allblocks && dmm->allvars && dmm->nosides && dmm->nounsides && dmm->nocontacts && dmm->nouncontacts) */
      /* DMCreateGlobalVector is defined() */
      Vec v;
      PetscInt low, high;

      ierr = DMCreateGlobalVector(dm,&v);CHKERRQ(ierr);
      ierr = VecGetOwnershipRange(v,&low,&high);CHKERRQ(ierr);
      ierr = ISCreateStride(((PetscObject)dm)->comm,(high-low),low,1,&dmm->embedding);CHKERRQ(ierr);
    }
  }
  ierr = PetscObjectReference((PetscObject)(dmm->embedding));CHKERRQ(ierr);
  *embedding = dmm->embedding;

  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMCreateFieldDecomposition_Moose"
static PetscErrorCode  DMCreateFieldDecomposition_Moose(DM dm, PetscInt *len, char ***namelist, IS **islist, DM **dmlist)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose *)(dm->data);

  PetscFunctionBegin;
  /* Only called after DMSetUp(). */
  if(!dmm->splitlocs) PetscFunctionReturn(0);
  *len = dmm->splitlocs->size();
  if(namelist) {ierr = PetscMalloc(*len*sizeof(char*),namelist);CHKERRQ(ierr);}
  if(islist)   {ierr = PetscMalloc(*len*sizeof(IS),islist);CHKERRQ(ierr);}
  if(dmlist)   {ierr = PetscMalloc(*len*sizeof(DM),dmlist);CHKERRQ(ierr);}
  for (std::multimap<std::string, unsigned int>::const_iterator dit = dmm->splitlocs->begin(); dit != dmm->splitlocs->end(); ++dit) {
    unsigned int                             d = dit->second;
    std::string                          dname = dit->first;
    DM_Moose::SplitInfo&                 dinfo = (*dmm->splits)[dname];
    if (!dinfo.dm) {
      ierr = DMCreateMoose(((PetscObject)dm)->comm, *dmm->nl, &dinfo.dm);CHKERRQ(ierr);
      ierr = PetscObjectSetOptionsPrefix((PetscObject)dinfo.dm,((PetscObject)dm)->prefix);CHKERRQ(ierr);
      std::string suffix = std::string("fieldsplit_")+dname+"_";
      ierr = PetscObjectAppendOptionsPrefix((PetscObject)dinfo.dm,suffix.c_str());CHKERRQ(ierr);
    }
    ierr = DMSetFromOptions(dinfo.dm);CHKERRQ(ierr);
    ierr = DMSetUp(dinfo.dm);CHKERRQ(ierr);
   if(namelist) {
      ierr = PetscStrallocpy(dname.c_str(),(*namelist)+d);CHKERRQ(ierr);
    }
    if(islist) {
      if (!dinfo.rembedding) {
	IS dembedding, lembedding;
	ierr = DMMooseGetEmbedding_Private(dinfo.dm,&dembedding);CHKERRQ(ierr);
	if(dmm->embedding) {
	  /* Create a relative embedding into the parent's index space. */
#if PETSC_VERSION_LE(3,3,0) && PETSC_VERSION_RELEASE
	  ierr = ISMapFactorRight(dembedding,dmm->embedding, PETSC_TRUE, &lembedding); CHKERRQ(ierr);
#else
	  ierr = ISEmbed(dembedding,dmm->embedding, PETSC_TRUE, &lembedding); CHKERRQ(ierr);
#endif
	  const PetscInt *lindices;
	  PetscInt len,dlen,llen,*rindices,off,i;
	  ierr = ISGetLocalSize(dembedding, &dlen); CHKERRQ(ierr);
	  ierr = ISGetLocalSize(lembedding, &llen); CHKERRQ(ierr);
	  if(llen != dlen) SETERRQ1(((PetscObject)dm)->comm, PETSC_ERR_PLIB, "Failed to embed split %D", d);
	  ierr = ISDestroy(&dembedding); CHKERRQ(ierr);
	  // Convert local embedding to global (but still relative) embedding
	  ierr = PetscMalloc(llen*sizeof(PetscInt),&rindices);CHKERRQ(ierr);
	  ierr = ISGetIndices(lembedding, &lindices);CHKERRQ(ierr);
	  ierr = PetscMemcpy(rindices,lindices,llen*sizeof(PetscInt));CHKERRQ(ierr);
	  ierr = ISDestroy(&lembedding); CHKERRQ(ierr);
	  // We could get the index offset from a corresponding global vector, but subDMs don't yet have global vectors
	  ierr = ISGetLocalSize(dmm->embedding,&len);CHKERRQ(ierr);
	  ierr = MPI_Scan(&len,&off,1,MPI_INT,MPI_SUM,((PetscObject)dm)->comm);CHKERRQ(ierr);
	  off -= len;
	  for (i = 0; i < llen; ++i) rindices[i] += off;
	  ierr = ISCreateGeneral(((PetscObject)dm)->comm,llen,rindices,PETSC_OWN_POINTER,&(dinfo.rembedding));CHKERRQ(ierr);
	}
	else {
	  dinfo.rembedding = dembedding;
	}
      }
      ierr = PetscObjectReference((PetscObject)(dinfo.rembedding));CHKERRQ(ierr);
      (*islist)[d] = dinfo.rembedding;
    }
    if (dmlist) {
      ierr = PetscObjectReference((PetscObject)dinfo.dm);CHKERRQ(ierr);
      (*dmlist)[d] = dinfo.dm;
    }
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMCreateDomainDecomposition_Moose"
static PetscErrorCode  DMCreateDomainDecomposition_Moose(DM dm, PetscInt *len, char ***namelist, IS **innerislist, IS **outerislist, DM **dmlist)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  /* Use DMCreateFieldDecomposition_Moose() to obtain everything but outerislist, which is currently PETSC_NULL. */
  if(outerislist)   *outerislist = PETSC_NULL; /* FIX: allow mesh-based overlap. */
  ierr = DMCreateFieldDecomposition_Moose(dm,len,namelist,innerislist,dmlist);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}



#if PETSC_VERSION_LE(3,3,0) && PETSC_VERSION_RELEASE
#undef  __FUNCT__
#define __FUNCT__ "DMCreateFieldDecompositionDM_Moose"
PetscErrorCode DMCreateFieldDecompositionDM_Moose(DM dm, const char*/*name*/,DM* ddm)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  /* Return self. */
  if (*ddm) {
    ierr = PetscObjectReference((PetscObject)dm);CHKERRQ(ierr);
    *ddm = dm;
  }
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMCreateDomainDecompositionDM_Moose"
PetscErrorCode DMCreateDomainDecompositionDM_Moose(DM dm, const char*/*name*/,DM* ddm)
{
  PetscErrorCode ierr;
  PetscBool ismoose;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  /* Return self. */
  if (*ddm) {
    ierr = PetscObjectReference((PetscObject)dm);CHKERRQ(ierr);
    *ddm = dm;
  }
  PetscFunctionReturn(0);
}
#endif


#undef __FUNCT__
#define __FUNCT__ "DMMooseFunction"
static PetscErrorCode DMMooseFunction(DM dm, Vec x, Vec r)
{
  PetscErrorCode ierr;
  PetscFunctionBegin;
  libmesh_assert(x);
  libmesh_assert(r);

  NonlinearSystem* nl;
  ierr = DMMooseGetNonlinearSystem(dm, nl); CHKERRQ(ierr);
  PetscVector<Number>& X_sys = *libmesh_cast_ptr<PetscVector<Number>* >(nl->sys().solution.get());
  PetscVector<Number>& R_sys = *libmesh_cast_ptr<PetscVector<Number>* >(nl->sys().rhs);
  PetscVector<Number> X_global(x, libMesh::CommWorld), R(r, libMesh::CommWorld);

  // Use the systems update() to get a good local version of the parallel solution
  X_global.swap(X_sys);
  R.swap(R_sys);

  nl->sys().get_dof_map().enforce_constraints_exactly(nl->sys());
  nl->sys().update();

  // Swap back
  X_global.swap(X_sys);
  R.swap(R_sys);
  R.zero();

  // if the user has provided both function pointers and objects only the pointer
  // will be used, so catch that as an error
  if (nl->sys().nonlinear_solver->residual && nl->sys().nonlinear_solver->residual_object) {
    std::ostringstream err;
    err << "ERROR: cannot specifiy both a function and object to compute the Residual!" << std::endl;
    mooseError(err.str());
  }
  if (nl->sys().nonlinear_solver->matvec && nl->sys().nonlinear_solver->residual_and_jacobian_object) {
    std::ostringstream err;
    err << "ERROR: cannot specifiy both a function and object to compute the combined Residual & Jacobian!" << std::endl;
    mooseError(err.str());
  }
  if (nl->sys().nonlinear_solver->residual != NULL) {
    nl->sys().nonlinear_solver->residual(*(nl->sys().current_local_solution.get()), R, nl->sys());
  }
  else if (nl->sys().nonlinear_solver->residual_object != NULL) {
    nl->sys().nonlinear_solver->residual_object->residual(*(nl->sys().current_local_solution.get()), R, nl->sys());
  }
  else if (nl->sys().nonlinear_solver->matvec   != NULL) {
    nl->sys().nonlinear_solver->matvec(*(nl->sys().current_local_solution.get()), &R, NULL, nl->sys());
  }
  else if (nl->sys().nonlinear_solver->residual_and_jacobian_object != NULL) {
    nl->sys().nonlinear_solver->residual_and_jacobian_object->residual_and_jacobian(*(nl->sys().current_local_solution.get()), &R, NULL, nl->sys());
  }
  else {
    std::ostringstream err;
    err << "No suitable residual computation routine found";
    mooseError(err.str());
  }
  R.close();
  X_global.close();
  PetscFunctionReturn(0);
}

#if !PETSC_VERSION_LE(3,3,0) || !PETSC_VERSION_RELEASE
#undef __FUNCT__
#define __FUNCT__ "SNESFunction_DMMoose"
static PetscErrorCode SNESFunction_DMMoose(SNES, Vec x, Vec r, void *ctx)
{
  DM dm = (DM)ctx;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DMMooseFunction(dm,x,r);CHKERRQ(ierr);
  PetscFunctionReturn(0);

}
#endif


#undef __FUNCT__
#define __FUNCT__ "DMMooseJacobian"
static PetscErrorCode DMMooseJacobian(DM dm, Vec x, Mat jac, Mat pc, MatStructure *msflag)
{
  PetscErrorCode ierr;
  NonlinearSystem *nl;

  PetscFunctionBegin;
  ierr = DMMooseGetNonlinearSystem(dm, nl);CHKERRQ(ierr);

  PetscMatrix<Number>  the_pc(pc);
  PetscMatrix<Number>  Jac(jac);
  PetscVector<Number>& X_sys = *libmesh_cast_ptr<PetscVector<Number>*>(nl->sys().solution.get());
  PetscMatrix<Number>& Jac_sys = *libmesh_cast_ptr<PetscMatrix<Number>*>(nl->sys().matrix);
  PetscVector<Number>  X_global(x, libMesh::CommWorld);

  // Set the dof maps
  the_pc.attach_dof_map(nl->sys().get_dof_map());
  Jac.attach_dof_map(nl->sys().get_dof_map());

  // Use the systems update() to get a good local version of the parallel solution
  X_global.swap(X_sys);
  Jac.swap(Jac_sys);

  nl->sys().get_dof_map().enforce_constraints_exactly(nl->sys());
  nl->sys().update();

  X_global.swap(X_sys);
  Jac.swap(Jac_sys);

  the_pc.zero();

  // if the user has provided both function pointers and objects only the pointer
  // will be used, so catch that as an error
  if (nl->sys().nonlinear_solver->jacobian && nl->sys().nonlinear_solver->jacobian_object) {
    std::ostringstream err;
    err << "ERROR: cannot specifiy both a function and object to compute the Jacobian!" << std::endl;
    mooseError(err.str());
  }
  if (nl->sys().nonlinear_solver->matvec && nl->sys().nonlinear_solver->residual_and_jacobian_object) {
    std::ostringstream err;
    err << "ERROR: cannot specifiy both a function and object to compute the combined Residual & Jacobian!" << std::endl;
    mooseError(err.str());
  }
  if (nl->sys().nonlinear_solver->jacobian != NULL) {
    nl->sys().nonlinear_solver->jacobian(*(nl->sys().current_local_solution.get()), the_pc, nl->sys());
  }
  else if (nl->sys().nonlinear_solver->jacobian_object != NULL) {
    nl->sys().nonlinear_solver->jacobian_object->jacobian(*(nl->sys().current_local_solution.get()), the_pc, nl->sys());
  }
  else if (nl->sys().nonlinear_solver->matvec != NULL) {
    nl->sys().nonlinear_solver->matvec(*(nl->sys().current_local_solution.get()), NULL, &the_pc, nl->sys());
  }
  else if (nl->sys().nonlinear_solver->residual_and_jacobian_object != NULL) {
    nl->sys().nonlinear_solver->residual_and_jacobian_object->residual_and_jacobian(*(nl->sys().current_local_solution.get()), NULL, &the_pc, nl->sys());
  }
  else {
    std::ostringstream err;
    err << "No suitable Jacobian routine or object";
    mooseError(err.str());
  }
  the_pc.close();
  Jac.close();
  X_global.close();

  *msflag = SAME_NONZERO_PATTERN;
  PetscFunctionReturn(0);
}

#if !PETSC_VERSION_LE(3,3,0) || !PETSC_VERSION_RELEASE
#undef  __FUNCT__
#define __FUNCT__ "SNESJacobian_DMMoose"
static PetscErrorCode SNESJacobian_DMMoose(SNES,Vec x,Mat *jac,Mat *pc, MatStructure* flag, void* ctx)
{
  DM dm = (DM)ctx;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DMMooseJacobian(dm,x,*jac,*pc,flag); CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
#endif

#undef __FUNCT__
#define __FUNCT__ "DMVariableBounds_Moose"
static PetscErrorCode DMVariableBounds_Moose(DM dm, Vec xl, Vec xu)
{
  PetscErrorCode ierr;
  NonlinearSystem* nl;
  PetscVector<Number> XL(xl, libMesh::CommWorld);
  PetscVector<Number> XU(xu, libMesh::CommWorld);

  PetscFunctionBegin;
  ierr = DMMooseGetNonlinearSystem(dm, nl);CHKERRQ(ierr);
  ierr = VecSet(xl, SNES_VI_NINF);CHKERRQ(ierr);
  ierr = VecSet(xu, SNES_VI_INF);CHKERRQ(ierr);
  if (nl->sys().nonlinear_solver->bounds != NULL) {
    nl->sys().nonlinear_solver->bounds(XL,XU,nl->sys());
  }
  else if (nl->sys().nonlinear_solver->bounds_object != NULL) {
    nl->sys().nonlinear_solver->bounds_object->bounds(XL,XU, nl->sys());
  }
  else {
    SETERRQ(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "No bounds calculation in this Moose object");
  }
  PetscFunctionReturn(0);

}


#undef __FUNCT__
#define __FUNCT__ "DMCreateGlobalVector_Moose"
static PetscErrorCode DMCreateGlobalVector_Moose(DM dm, Vec *x)
{
  PetscErrorCode ierr;
  DM_Moose      *dmm = (DM_Moose *)(dm->data);
  PetscBool      ismoose;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose); CHKERRQ(ierr);
  if (!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "DM of type %s, not of type %s", ((PetscObject)dm)->type, DMMOOSE);
  if (!dmm->nl) SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");

  NumericVector<Number>* nv = (dmm->nl->sys().solution).get();
  PetscVector<Number>*   pv = dynamic_cast<PetscVector<Number>*>(nv);
  Vec                    v  = pv->vec();
  /* Unfortunately, currently this does not produce a ghosted vector, so nonlinear subproblem solves aren't going to be easily available.
     Should work fine for getting vectors out for linear subproblem solvers. */
  if(dmm->embedding) {
    PetscInt n;
    ierr = VecCreate(((PetscObject)v)->comm, x);CHKERRQ(ierr);
    ierr = ISGetLocalSize(dmm->embedding, &n);CHKERRQ(ierr);
    ierr = VecSetSizes(*x,n,PETSC_DETERMINE);CHKERRQ(ierr);
    ierr = VecSetType(*x,((PetscObject)v)->type_name);CHKERRQ(ierr);
    ierr = VecSetFromOptions(*x);CHKERRQ(ierr);
    ierr = VecSetUp(*x);CHKERRQ(ierr);
  }
  else {
    ierr = VecDuplicate(v,x);CHKERRQ(ierr);
  }
  ierr = PetscObjectCompose((PetscObject)*x,"DM",(PetscObject)dm); CHKERRQ(ierr);
  PetscFunctionReturn(0);

}


#undef __FUNCT__
#define __FUNCT__ "DMCreateMatrix_Moose"
static PetscErrorCode DMCreateMatrix_Moose(DM dm, const MatType type, Mat *A)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose *)(dm->data);
  PetscBool      ismoose;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);CHKERRQ(ierr);
  if (!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "DM of type %s, not of type %s", ((PetscObject)dm)->type, DMMOOSE);
  if (!dmm->nl) SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");
  /*
    The simplest thing for now: compute the sparsity_pattern using dof_map and init the matrix using that info.
    TODO: compute sparsity restricted to this DM's blocks, variables and sides.
    Even fancier: compute the sparsity of the coupling of a contact slave to the contact master.
    In any event, here we are in control of the matrix type and structure.
  */
  DofMap& dof_map = dmm->nl->sys().get_dof_map();
  PetscInt M,N,m,n;
  MPI_Comm comm;
  M = dof_map.n_dofs();
  N = M;
  m = dof_map.n_dofs_on_processor(dmm->nl->sys().processor_id());
  n = m;
  ierr = PetscObjectGetComm((PetscObject)dm,&comm);CHKERRQ(ierr);
  ierr = MatCreate(comm, A);CHKERRQ(ierr);
  ierr = MatSetSizes(*A,m,n,M,N);CHKERRQ(ierr);
  ierr = MatSetType(*A,type);CHKERRQ(ierr);
  /* Set preallocation for the basic sparse matrix types (applies only if *A has the right type. */
  /* For now we ignore blocksize issues, since BAIJ doesn't play well with field decomposition by variable. */
  const std::vector<numeric_index_type>& n_nz = dof_map.get_n_nz();
  const std::vector<numeric_index_type>& n_oz = dof_map.get_n_oz();
  ierr = MatSeqAIJSetPreallocation(*A, 0, (PetscInt*)(n_nz.empty()?NULL:&n_nz[0]));CHKERRQ(ierr);
  ierr = MatMPIAIJSetPreallocation(*A, 0, (PetscInt*)(n_nz.empty()?NULL:&n_nz[0]),0, (PetscInt*)(n_oz.empty()?NULL:&n_oz[0]));CHKERRQ(ierr);
  /* TODO: set the prefix for *A and MatSetFromOptions(*A)? Might override the type and other settings made here. */
  ierr = MatSetUp(*A);CHKERRQ(ierr);
  PetscFunctionReturn(0);

}


#undef __FUNCT__
#define __FUNCT__ "DMView_Moose"
static PetscErrorCode  DMView_Moose(DM dm, PetscViewer viewer)
{
  PetscErrorCode ierr;
  PetscBool      isascii;
  const char     *name, *prefix;
  DM_Moose       *dmm = (DM_Moose*)dm->data;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)viewer,PETSCVIEWERASCII,&isascii); CHKERRQ(ierr);
  if(isascii) {
    ierr = PetscObjectGetName((PetscObject)dm, &name);     CHKERRQ(ierr);
    ierr = PetscObjectGetOptionsPrefix((PetscObject)dm, &prefix); CHKERRQ(ierr);
    ierr = PetscViewerASCIIPrintf(viewer, "DM Moose with name %s and prefix %s\n", name, prefix); CHKERRQ(ierr);
    ierr = PetscViewerASCIIPrintf(viewer, "variables:", name, prefix); CHKERRQ(ierr);
    std::map<std::string,unsigned int>::iterator vit = dmm->varids->begin();
    std::map<std::string,unsigned int>::const_iterator vend = dmm->varids->end();
    for(; vit != vend; ++vit) {
      ierr = PetscViewerASCIIPrintf(viewer, "(%s,%D) ", vit->first.c_str(), vit->second); CHKERRQ(ierr);
    }
    ierr = PetscViewerASCIIPrintf(viewer, "\n");CHKERRQ(ierr);
    ierr = PetscViewerASCIIPrintf(viewer, "blocks:"); CHKERRQ(ierr);
    std::map<std::string,unsigned int>::iterator bit = dmm->blockids->begin();
    std::map<std::string,unsigned int>::const_iterator bend = dmm->blockids->end();
    for(; bit != bend; ++bit) {
      ierr = PetscViewerASCIIPrintf(viewer, "(%s,%D) ", bit->first.c_str(), bit->second); CHKERRQ(ierr);
    }
    ierr = PetscViewerASCIIPrintf(viewer, "\n");CHKERRQ(ierr);
    if (dmm->sideids->size()) {
      ierr = PetscViewerASCIIPrintf(viewer, "sides:"); CHKERRQ(ierr);
      std::map<std::string,BoundaryID>::iterator sit = dmm->sideids->begin();
      std::map<std::string,BoundaryID>::const_iterator send = dmm->sideids->end();
      for (; sit != send; ++sit) {
	ierr = PetscViewerASCIIPrintf(viewer, "(%s,%D) ", sit->first.c_str(), sit->second);CHKERRQ(ierr);
      }
      ierr = PetscViewerASCIIPrintf(viewer, "\n");CHKERRQ(ierr);
    }
    if (dmm->unsideids->size()) {
      ierr = PetscViewerASCIIPrintf(viewer, "unsides:");CHKERRQ(ierr);
      std::map<std::string,BoundaryID>::iterator sit = dmm->unsideids->begin();
      std::map<std::string,BoundaryID>::const_iterator send = dmm->unsideids->end();
      for (; sit != send; ++sit) {
	ierr = PetscViewerASCIIPrintf(viewer, "(%s,%D) ", sit->first.c_str(), sit->second);CHKERRQ(ierr);
      }
      ierr = PetscViewerASCIIPrintf(viewer, "\n");CHKERRQ(ierr);
    }
    if (dmm->contactnames->size()) {
      ierr = PetscViewerASCIIPrintf(viewer, "contacts:");CHKERRQ(ierr);
      for (std::map<DM_Moose::ContactID,DM_Moose::ContactName>::iterator cit = dmm->contactnames->begin(); cit != dmm->contactnames->end(); ++cit) {
	ierr = PetscViewerASCIIPrintf(viewer, "(%s,%s,", cit->second.first.c_str(), cit->second.second.c_str());CHKERRQ(ierr);
	if ((*dmm->contact_displaced)[cit->second]) {
	  ierr = PetscViewerASCIIPrintf(viewer, "displaced) ", cit->second.first.c_str(), cit->second.second.c_str());CHKERRQ(ierr);
	} else {
	  ierr = PetscViewerASCIIPrintf(viewer, "undisplaced) ", cit->second.first.c_str(), cit->second.second.c_str());CHKERRQ(ierr);
	}
      }
      ierr = PetscViewerASCIIPrintf(viewer, "\n");CHKERRQ(ierr);
    }
    if (dmm->uncontactnames->size()) {
      ierr = PetscViewerASCIIPrintf(viewer, "uncontacts:");CHKERRQ(ierr);
      for (std::map<DM_Moose::ContactID,DM_Moose::ContactName>::iterator cit = dmm->uncontactnames->begin(); cit != dmm->uncontactnames->end(); ++cit) {
	ierr = PetscViewerASCIIPrintf(viewer, "(%s,%s,", cit->second.first.c_str(), cit->second.second.c_str());CHKERRQ(ierr);
	if ((*dmm->uncontact_displaced)[cit->second]) {
	  ierr = PetscViewerASCIIPrintf(viewer, "displaced) ", cit->second.first.c_str(), cit->second.second.c_str());CHKERRQ(ierr);
	} else {
	  ierr = PetscViewerASCIIPrintf(viewer, "undisplaced) ", cit->second.first.c_str(), cit->second.second.c_str());CHKERRQ(ierr);
	}
      }
      ierr = PetscViewerASCIIPrintf(viewer, "\n");CHKERRQ(ierr);
    }
    if (dmm->splitlocs && dmm->splitlocs->size()) {
      ierr = PetscViewerASCIIPrintf(viewer, "Field decomposition:");CHKERRQ(ierr);
      /* FIX: decompositions might have different sizes and components on different ranks. */
      for (std::multimap<std::string, unsigned int>::const_iterator dit = dmm->splitlocs->begin(); dit != dmm->splitlocs->end(); ++dit) {
	std::string dname = dit->first;
	ierr = PetscViewerASCIIPrintf(viewer, " %s", dname.c_str());CHKERRQ(ierr);
      }
      ierr = PetscViewerASCIIPrintf(viewer, "\n");CHKERRQ(ierr);
    }
  } else {
    SETERRQ(PETSC_COMM_SELF, PETSC_ERR_SUP, "Non-ASCII viewers are not supported");
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseGetMeshBlocks_Private"
static PetscErrorCode  DMMooseGetMeshBlocks_Private(DM dm, std::set<subdomain_id_type>& blocks)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose *)(dm->data);
  PetscBool      ismoose;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose); CHKERRQ(ierr);
  if (!ismoose)  SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "DM of type %s, not of type %s", ((PetscObject)dm)->type, DMMOOSE);
  if (!dmm->nl) SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");

  const MeshBase& mesh = dmm->nl->sys().get_mesh();
  /* The following effectively is a verbatim copy of MeshBase::n_subdomains(). */
  // This requires an inspection on every processor
  parallel_only();
  MeshBase::const_element_iterator       el  = mesh.active_elements_begin();
  const MeshBase::const_element_iterator end = mesh.active_elements_end();
  for (; el!=end; ++el)
    blocks.insert((*el)->subdomain_id());
  // Some subdomains may only live on other processors
  CommWorld.set_union(blocks);
  PetscFunctionReturn(0);
}


#undef __FUNCT__
#define __FUNCT__ "DMSetUp_Moose_Pre"
static PetscErrorCode  DMSetUp_Moose_Pre(DM dm)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose *)(dm->data);
  PetscBool      ismoose;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose); CHKERRQ(ierr);
  if (!ismoose)  SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "DM of type %s, not of type %s", ((PetscObject)dm)->type, DMMOOSE);
  if (!dmm->nl) SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");

  /* Set up variables, blocks and sides. */
  DofMap& dofmap = dmm->nl->sys().get_dof_map();
  /* libMesh mesh */
  const MeshBase& mesh = dmm->nl->sys().get_mesh();

  dmm->nosides = PETSC_TRUE;
  dmm->sideids->clear();
  dmm->sidenames->clear();
  if (dmm->sides) {
    dmm->nosides = PETSC_FALSE;
    std::set<BoundaryID> ids;
    for (std::set<std::string>::iterator nameit = dmm->sides->begin(); nameit != dmm->sides->end(); ++nameit) {
      std::string name = *nameit;
      boundary_id_type id = dmm->nl->mesh().getBoundaryID(name);
      dmm->sidenames->insert(std::pair<boundary_id_type,std::string>(id,name));
      dmm->sideids->insert(std::pair<std::string,boundary_id_type>(name,id));
    }
    delete dmm->sides;
    dmm->sides = PETSC_NULL;
  }
  dmm->nounsides = PETSC_TRUE;
  dmm->unsideids->clear();
  dmm->unsidenames->clear();
  if (dmm->unsides) {
    dmm->nounsides = PETSC_FALSE;
    std::set<BoundaryID> ids;
    for (std::set<std::string>::iterator nameit = dmm->unsides->begin(); nameit != dmm->unsides->end(); ++nameit) {
      std::string name = *nameit;
      boundary_id_type id = dmm->nl->mesh().getBoundaryID(name);
      dmm->unsidenames->insert(std::pair<boundary_id_type,std::string>(id,name));
      dmm->unsideids->insert(std::pair<std::string,boundary_id_type>(name,id));
    }
    delete dmm->unsides;
    dmm->unsides = PETSC_NULL;
  }
  dmm->nocontacts = PETSC_TRUE;
  if (dmm->contacts) {
    dmm->nocontacts = PETSC_FALSE;
    for (std::set<DM_Moose::ContactName>::iterator cit = dmm->contacts->begin(); cit != dmm->contacts->end(); ++cit) {
      try {
	dmm->nl->_fe_problem.geomSearchData().getPenetrationLocator(cit->second,cit->first);
      }
      catch(...) {
	std::ostringstream err;
	err << "Problem retrieving contact for PenetrationLocator with master " << cit->first << " and slave " << cit->second;
	mooseError(err.str());
      }
      BoundaryID master_id = dmm->nl->mesh().getBoundaryID(cit->first);
      BoundaryID slave_id = dmm->nl->mesh().getBoundaryID(cit->second);
      DM_Moose::ContactID  cid(master_id,slave_id);
      dmm->contactnames->insert(std::pair<DM_Moose::ContactID,DM_Moose::ContactName>(cid,*cit));
    }
  }
  dmm->nouncontacts = PETSC_TRUE;
  if (dmm->uncontacts) {
    dmm->nouncontacts = PETSC_FALSE;
    for (std::set<DM_Moose::ContactName>::iterator cit = dmm->uncontacts->begin(); cit != dmm->uncontacts->end(); ++cit) {
      try {
	dmm->nl->_fe_problem.geomSearchData().getPenetrationLocator(cit->first,cit->second);
      }
      catch(...) {
	std::ostringstream err;
	err << "Problem retrieving uncontact for PenetrationLocator with master " << cit->first << " and slave " << cit->second;
	mooseError(err.str());
      }
      BoundaryID master_id = dmm->nl->mesh().getBoundaryID(cit->first);
      BoundaryID slave_id = dmm->nl->mesh().getBoundaryID(cit->second);
      DM_Moose::ContactID   cid(master_id,slave_id);
      dmm->uncontactnames->insert(std::pair<DM_Moose::ContactID,DM_Moose::ContactName>(cid,*cit));
    }
  }

  dmm->varids->clear();
  dmm->varnames->clear();
  // FIXME: would be nice to invert this nested loop structure so we could iterate over the potentially smaller dmm->vars,
  // but checking against dofmap.variable would still require a linear search, hence, no win.  Would be nice to endow dofmap.variable
  // with a fast search capability.
  for(unsigned int v = 0; v < dofmap.n_variables(); ++v) {
    std::string vname = dofmap.variable(v).name();
    if (dmm->vars && dmm->vars->size() && dmm->vars->find(vname) == dmm->vars->end()) continue;
    dmm->varids->insert(std::pair<std::string,unsigned int>(vname,v));
    dmm->varnames->insert(std::pair<unsigned int,std::string>(v,vname));
  }
  if (dmm->varids->size() == dofmap.n_variables()) dmm->allvars = PETSC_TRUE; else dmm->allvars = PETSC_FALSE;
  if (dmm->vars) {
    delete dmm->vars;
    dmm->vars = PETSC_NULL;
  }

  dmm->blockids->clear();
  dmm->blocknames->clear();
  std::set<subdomain_id_type> blocks;
  ierr = DMMooseGetMeshBlocks_Private(dm,blocks);CHKERRQ(ierr);
  std::set<subdomain_id_type>::iterator bit = blocks.begin();
  std::set<subdomain_id_type>::iterator bend = blocks.end();
  if(bit == bend) SETERRQ(((PetscObject)dm)->comm, PETSC_ERR_PLIB, "No mesh blocks found.");
  for(; bit != bend; ++bit) {
    subdomain_id_type bid = *bit;
    std::string bname = mesh.subdomain_name(bid);
    if(!bname.length()) {
      /* Block names are currently implemented for Exodus II meshes
	 only, so we might have to make up our own block names and
	 maintain our own mapping of block ids to names.
      */
      std::ostringstream ss;
      ss << bid;
      bname = ss.str();
    }
    if (dmm->nosides) {
      // If no sides have been specified, by default (null or empty dmm->blocks) all blocks are included in the split
      // Thus, skip this block only if it is explicitly excluded from a nonempty dmm->blocks.
      if (dmm->blocks && dmm->blocks->size() && dmm->blocks->find(bname) == dmm->blocks->end()) continue;
    } else {
      // If sides have been specified, only the explicitly-specified blocks (those in dmm->blocks, if it's non-null) are in the split.
      // Thus, include this block only if it is explicitly specified in a nonempty dmm->blocks.
      // Equivalently, skip this block if dmm->blocks is dmm->blocks is null or empty or excludes this block.
      if (!dmm->blocks || !dmm->blocks->size() || dmm->blocks->find(bname) == dmm->blocks->end()) continue;
    }
    dmm->blockids->insert(std::pair<std::string,unsigned int>(bname,bid));
    dmm->blocknames->insert(std::pair<unsigned int,std::string>(bid,bname));
  }
  if (dmm->blockids->size() == blocks.size()) dmm->allblocks = PETSC_TRUE; else dmm->allblocks = PETSC_FALSE;
  if (dmm->blocks) {
    delete dmm->blocks;
    dmm->blocks = PETSC_NULL;
  }

  std::string name = dmm->nl->sys().name();
  name += "_vars";
  for (std::map<unsigned int, std::string>::const_iterator vit = dmm->varnames->begin(); vit != dmm->varnames->end(); ++vit) {
    name += "_"+vit->second;
  }
  name += "_blocks";
  for (std::map<unsigned int, std::string>::const_iterator bit = dmm->blocknames->begin(); bit != dmm->blocknames->end(); ++bit) {
    name += "_"+bit->second;
  }
  if (dmm->sidenames && dmm->sidenames->size()) {
    name += "_sides";
    for (std::map<BoundaryID, std::string>::const_iterator sit = dmm->sidenames->begin(); sit != dmm->sidenames->end(); ++sit) {
      name += "_"+sit->second;
    }
  }
  if (dmm->unsidenames && dmm->unsidenames->size()) {
    name += "_unsides";
    for (std::map<BoundaryID, std::string>::const_iterator sit = dmm->unsidenames->begin(); sit != dmm->unsidenames->end(); ++sit) {
      name += "_"+sit->second;
    }
  }
  if (dmm->contactnames && dmm->contactnames->size()) {
    name += "_contacts";
    for (std::map<DM_Moose::ContactID, DM_Moose::ContactName>::const_iterator cit = dmm->contactnames->begin(); cit != dmm->contactnames->end(); ++cit) {
      name += "_master_"+cit->second.first+"_slave_"+cit->second.second;
    }
  }
  if (dmm->uncontactnames && dmm->uncontactnames->size()) {
    name += "_uncontacts";
    for (std::map<DM_Moose::ContactID, DM_Moose::ContactName>::const_iterator cit = dmm->uncontactnames->begin(); cit != dmm->uncontactnames->end(); ++cit) {
      name += "_master_"+cit->second.first+"_slave_"+cit->second.second;
    }
  }
  ierr = PetscObjectSetName((PetscObject)dm,name.c_str());CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMooseReset"
PetscErrorCode  DMMooseReset(DM dm)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose *)(dm->data);
  PetscBool      ismoose;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose); CHKERRQ(ierr);
  if (!ismoose)  PetscFunctionReturn(0);
  if (!dmm->nl) SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");
  ierr = ISDestroy(&dmm->embedding);CHKERRQ(ierr);
  for (std::map<std::string,DM_Moose::SplitInfo>::iterator it = dmm->splits->begin(); it != dmm->splits->end(); ++it) {
    DM_Moose::SplitInfo& split = it->second;
    ierr = ISDestroy(&split.rembedding);CHKERRQ(ierr);
    if (split.dm) {
      ierr = DMMooseReset(split.dm);CHKERRQ(ierr);
    }
  }
  dm->setupcalled = PETSC_FALSE;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMSetUp_Moose"
static PetscErrorCode  DMSetUp_Moose(DM dm)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose *)(dm->data);
  PetscBool      ismoose;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose); CHKERRQ(ierr);
  if (!ismoose)  SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "DM of type %s, not of type %s", ((PetscObject)dm)->type, DMMOOSE);
  if (!dmm->nl) SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");
  if (dmm->print_embedding) {
    const char *name, *prefix;
    IS embedding;

    ierr = PetscObjectGetName((PetscObject)dm, &name);     CHKERRQ(ierr);
    ierr = PetscObjectGetOptionsPrefix((PetscObject)dm, &prefix); CHKERRQ(ierr);
    ierr = PetscViewerASCIIPrintf(PETSC_VIEWER_STDOUT_(((PetscObject)dm)->comm), "DM Moose with name %s and prefix %s\n", name, prefix); CHKERRQ(ierr);
    if (dmm->allvars && dmm->allblocks && dmm->nosides && dmm->nounsides && dmm->nocontacts && dmm->nouncontacts) {
      ierr = PetscViewerASCIIPrintf(PETSC_VIEWER_STDOUT_(((PetscObject)dm)->comm), "\thas a trivial embedding\n");CHKERRQ(ierr);
    } else {
      ierr = DMMooseGetEmbedding_Private(dm,&embedding);CHKERRQ(ierr);
      ierr = PetscViewerASCIIPrintf(PETSC_VIEWER_STDOUT_(((PetscObject)dm)->comm), "\thas embedding defined by IS:\n");CHKERRQ(ierr);
      ierr = ISView(embedding, PETSC_VIEWER_STDOUT_(((PetscObject)dm)->comm));CHKERRQ(ierr);
      ierr = ISDestroy(&embedding);CHKERRQ(ierr);
    }
  }
  /*
     Do not evaluate function, Jacobian or bounds for an embedded DM -- the subproblem might not have enough information for that.
  */
  if (dmm->allvars && dmm->allblocks && dmm->nosides && dmm->nounsides && dmm->nocontacts && dmm->nouncontacts)  {
#if PETSC_VERSION_LE(3,3,0) && PETSC_VERSION_RELEASE
    ierr = DMSetFunction(dm, DMMooseFunction); CHKERRQ(ierr);
    ierr = DMSetJacobian(dm, DMMooseJacobian); CHKERRQ(ierr);
#else
    ierr = DMSNESSetFunction(dm, SNESFunction_DMMoose, (void*)dm); CHKERRQ(ierr);
    ierr = DMSNESSetJacobian(dm, SNESJacobian_DMMoose, (void*)dm); CHKERRQ(ierr);
#endif
    if (dmm->nl->sys().nonlinear_solver->bounds || dmm->nl->sys().nonlinear_solver->bounds_object)
      ierr = DMSetVariableBounds(dm, DMVariableBounds_Moose); CHKERRQ(ierr);
  }
  else {
    /*
       Fow now we don't implement even these, although a linear "Dirichlet" subproblem is well-defined.
       Creating the submatrix, however, might require extracting the submatrix preallocation from an unassembled matrix.
    */
      dm->ops->createglobalvector = 0;
      dm->ops->creatematrix = 0;
  }
  PetscFunctionReturn(0);

}


#undef __FUNCT__
#define __FUNCT__ "DMSetFromOptions_Moose"
PetscErrorCode  DMSetFromOptions_Moose(DM dm)
{
  PetscErrorCode ierr;
  PetscBool      ismoose;
  DM_Moose       *dmm = (DM_Moose*)dm->data;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose); CHKERRQ(ierr);
  if (!ismoose)  SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "DM of type %s, not of type %s", ((PetscObject)dm)->type, DMMOOSE);
  if (!dmm->nl) SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");
  ierr = PetscOptionsBegin(((PetscObject)dm)->comm, ((PetscObject)dm)->prefix, "DMMoose options", "DM");CHKERRQ(ierr);
  std::string opt, help;
  PetscInt maxvars = dmm->nl->sys().get_dof_map().n_variables();
  char** vars;
  std::set<std::string> varset;
  PetscInt nvars = maxvars;
  ierr = PetscMalloc(maxvars*sizeof(char*),&vars);CHKERRQ(ierr);
  opt = "-dm_moose_vars";
  help = "Variables in DMMoose";
  ierr = PetscOptionsStringArray(opt.c_str(),help.c_str(),"DMMooseSetVars",vars,&nvars,PETSC_NULL);CHKERRQ(ierr);
  for (PetscInt i = 0; i < nvars; ++i) {
    varset.insert(std::string(vars[i]));
    ierr = PetscFree(vars[i]);CHKERRQ(ierr);
  }
  ierr = PetscFree(vars);CHKERRQ(ierr);
  if (varset.size()) {
    ierr = DMMooseSetVariables(dm,varset);CHKERRQ(ierr);
  }
  //
  std::set<subdomain_id_type> meshblocks;
  ierr = DMMooseGetMeshBlocks_Private(dm,meshblocks);CHKERRQ(ierr);
  PetscInt maxblocks = meshblocks.size();
  char** blocks;
  ierr = PetscMalloc(maxblocks*sizeof(char*),&blocks);CHKERRQ(ierr);
  std::set<std::string> blockset;
  PetscInt nblocks = maxblocks;
  opt = "-dm_moose_blocks";
  help = "Blocks in DMMoose";
  ierr = PetscOptionsStringArray(opt.c_str(),help.c_str(),"DMMooseSetBlocks",blocks,&nblocks,PETSC_NULL);CHKERRQ(ierr);
  for (PetscInt i = 0; i < nblocks; ++i) {
    blockset.insert(std::string(blocks[i]));
    ierr = PetscFree(blocks[i]);CHKERRQ(ierr);
  }
  ierr = PetscFree(blocks);CHKERRQ(ierr);
  if (blockset.size()) {
    ierr = DMMooseSetBlocks(dm,blockset);CHKERRQ(ierr);
  }
  PetscInt maxsides = dmm->nl->sys().get_mesh().boundary_info->get_boundary_ids().size();
  char** sides;
  ierr = PetscMalloc(maxsides*sizeof(char*),&sides);CHKERRQ(ierr);
  PetscInt nsides = maxsides;
  std::set<std::string> sideset;
  opt = "-dm_moose_sides";
  help = "Sides to include in DMMoose";
  ierr = PetscOptionsStringArray(opt.c_str(),help.c_str(),"DMMooseSetSides",sides,&nsides,PETSC_NULL);CHKERRQ(ierr);
  for (PetscInt i = 0; i < nsides; ++i) {
    sideset.insert(std::string(sides[i]));
    ierr = PetscFree(sides[i]);CHKERRQ(ierr);
  }
  if (sideset.size()) {
    ierr = DMMooseSetSides(dm,sideset);CHKERRQ(ierr);
  }
  opt = "-dm_moose_unsides";
  help = "Sides to exclude from DMMoose";
  nsides = maxsides;
  ierr = PetscOptionsStringArray(opt.c_str(),help.c_str(),"DMMooseSetUnSides",sides,&nsides,PETSC_NULL);CHKERRQ(ierr);
  sideset.clear();
  for (PetscInt i = 0; i < nsides; ++i) {
    sideset.insert(std::string(sides[i]));
    ierr = PetscFree(sides[i]);CHKERRQ(ierr);
  }
  if (sideset.size()) {
    ierr = DMMooseSetUnSides(dm,sideset);CHKERRQ(ierr);
  }
  ierr = PetscFree(sides);CHKERRQ(ierr);
  PetscInt maxcontacts = dmm->nl->_fe_problem.geomSearchData()._penetration_locators.size();
  DisplacedProblem *displaced_problem = dmm->nl->_fe_problem.getDisplacedProblem();
  if (displaced_problem) {
    maxcontacts = PetscMax(maxcontacts,(PetscInt)displaced_problem->geomSearchData()._penetration_locators.size());
  }
  std::vector<DM_Moose::ContactName> contacts;
  std::vector<PetscBool> contact_displaced;
  PetscInt ncontacts = 0;
  opt = "-dm_moose_ncontacts";
  help = "Number of contacts to include in DMMoose.  For each <n> < dm_moose_contacts\n\t-dm_moose_contact_<n> is a comma-separated <master>,<slave> pair defining the contact surfaces"
    "\t-dm_moose_contact_<n>_displaced <bool> determines whether the contact is defined on the displaced mesh or not";
  ierr = PetscOptionsInt(opt.c_str(),help.c_str(),"DMMooseSetContacts",ncontacts,&ncontacts,PETSC_NULL);CHKERRQ(ierr);
  if (ncontacts > maxcontacts) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_SIZ, "Number of requested contacts %D exceeds the maximum number of contacts %D", ncontacts, maxcontacts);
  for (PetscInt i = 0; i < ncontacts; ++i) {
    {
      char* master_slave[2];
      PetscInt sz = 2;
      std::ostringstream oopt,ohelp;
      oopt << "-dm_moose_contact_" << i;
      ohelp << "Master and slave for contact " << i;
      const char *opts = oopt.str().c_str();
      const char *helps = ohelp.str().c_str();
      ierr = PetscOptionsStringArray(opts,helps,"DMMooseSetContacts",master_slave,&sz,PETSC_NULL);CHKERRQ(ierr);
      if (sz != 2) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_SIZ, "Expected 2 sideset IDs (master & slave) for contact %D, got %D instead", i, sz);
      contacts.push_back(DM_Moose::ContactName(std::string(master_slave[0]),std::string(master_slave[1])));
      ierr = PetscFree(master_slave[0]);CHKERRQ(ierr);
      ierr = PetscFree(master_slave[1]);CHKERRQ(ierr);
    }
    {
      PetscBool displaced = PETSC_FALSE;
      std::ostringstream oopt,ohelp;
      oopt << "-dm_moose_contact_" << i << "_displaced";
      ohelp << "Whether contact " << i << " is determined using displaced mesh or not";
      ierr = PetscOptionsBool(oopt.str().c_str(),ohelp.str().c_str(),"DMMooseSetContacts",PETSC_FALSE,&displaced,PETSC_NULL);CHKERRQ(ierr);
      contact_displaced.push_back(displaced);
    }
  }
  if (contacts.size()) {
    ierr = DMMooseSetContacts(dm,contacts,contact_displaced);CHKERRQ(ierr);
  }
  std::vector<DM_Moose::ContactName> uncontacts;
  std::vector<PetscBool> uncontact_displaced;
  PetscInt nuncontacts = 0;
  opt = "-dm_moose_nuncontacts";
  help = "Number of contacts to exclude from DMMoose.  For each <n> < dm_moose_contacts\n\t-dm_moose_contact_<n> is a comma-separated <master>,<slave> pair defining the contact surfaces"
    "\t-dm_moose_contact_<n>_displaced <bool> determines whether the contact is defined on the displaced mesh or not";
  ierr = PetscOptionsInt(opt.c_str(),help.c_str(),"DMMooseSetUnContacts",nuncontacts,&nuncontacts,PETSC_NULL);CHKERRQ(ierr);
  if (nuncontacts > maxcontacts) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_SIZ, "Number of requested uncontacts %D exceeds the maximum number of contacts %D", nuncontacts, maxcontacts);
  for (PetscInt i = 0; i < nuncontacts; ++i) {
    {
      char* master_slave[2];
      PetscInt sz = 2;
      std::ostringstream oopt,ohelp;
      oopt << "-dm_moose_uncontact_" << i;
      ohelp << "Master and slave for uncontact " << i;
      ierr = PetscOptionsStringArray(oopt.str().c_str(),ohelp.str().c_str(),"DMMooseSetUnContacts",master_slave,&sz,PETSC_NULL);CHKERRQ(ierr);
      if (sz != 2) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_SIZ, "Expected 2 sideset IDs (master & slave) for uncontact %D, got %D instead", i, sz);
      uncontacts.push_back(DM_Moose::ContactName(std::string(master_slave[0]),std::string(master_slave[1])));
      ierr = PetscFree(master_slave[0]);CHKERRQ(ierr);
      ierr = PetscFree(master_slave[1]);CHKERRQ(ierr);
    }
    {
      PetscBool displaced = PETSC_FALSE;
      std::ostringstream oopt,ohelp;
      oopt << "-dm_moose_uncontact_" << i << "_displaced";
      ohelp << "Whether uncontact " << i << " is determined using displaced mesh or not";
      ierr = PetscOptionsBool(oopt.str().c_str(),ohelp.str().c_str(),"DMMooseSetUnContact", PETSC_FALSE,&displaced,PETSC_NULL);CHKERRQ(ierr);
      uncontact_displaced.push_back(displaced);
    }
  }
  if (uncontacts.size()) {
    ierr = DMMooseSetUnContacts(dm,uncontacts,uncontact_displaced);CHKERRQ(ierr);
  }

  PetscInt nsplits = 0;
  /* Insert the usage of -dm_moose_fieldsplit_names into this help message, since the following if-clause might never fire, if -help is requested. */
  const char* fdhelp = "Number of named fieldsplits defined by the DM.\n\
                \tNames of fieldsplits are defined by -dm_moose_fieldsplit_names <splitname1> <splitname2> ...\n\
                \tEach split can be configured with its own variables, blocks and sides, as any DMMoose";
  ierr = PetscOptionsInt("-dm_moose_nfieldsplits", fdhelp, "DMMooseSetSplitNames", nsplits, &nsplits, NULL);CHKERRQ(ierr);
  if (nsplits) {
    PetscInt nnsplits = nsplits;
    std::vector<std::string> split_names;
    char** splitnames;
    ierr = PetscMalloc(nsplits*sizeof(char*),&splitnames);CHKERRQ(ierr);
    ierr = PetscOptionsStringArray("-dm_moose_fieldsplit_names","Names of fieldsplits defined by the DM","DMMooseSetSplitNames",splitnames,&nnsplits,PETSC_NULL);CHKERRQ(ierr);
    if (!nnsplits) {
      for (PetscInt i = 0; i < nsplits; ++i) {
	std::ostringstream s;
	s << i;
	split_names.push_back(s.str());
      }
    } else if (nsplits != nnsplits) {
      SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_SIZ, "Expected %D fieldsplit names, got %D instead", nsplits, nnsplits);
    } else {
      for (PetscInt i = 0; i < nsplits; ++i) {
	split_names.push_back(std::string(splitnames[i]));
	ierr = PetscFree(splitnames[i]);CHKERRQ(ierr);
      }
    }
    ierr = PetscFree(splitnames);CHKERRQ(ierr);
    ierr = DMMooseSetSplitNames(dm, split_names);CHKERRQ(ierr);
  }
  ierr = PetscOptionsBool("-dm_moose_print_embedding", "Print IS embedding DM's dofs", "DMMoose", dmm->print_embedding, &dmm->print_embedding, PETSC_NULL);CHKERRQ(ierr);
  ierr = PetscOptionsEnd();CHKERRQ(ierr);
  ierr = DMSetUp_Moose_Pre(dm);CHKERRQ(ierr); /* Need some preliminary set up because, strangely enough, DMView() is called in DMSetFromOptions(). */
  PetscFunctionReturn(0);
}


#undef __FUNCT__
#define __FUNCT__ "DMDestroy_Moose"
static PetscErrorCode  DMDestroy_Moose(DM dm)
{
  DM_Moose *dmm = (DM_Moose*)(dm->data);
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (dmm->vars) delete dmm->vars;
  delete dmm->varids;
  delete dmm->varnames;
  if (dmm->blocks) delete dmm->blocks;
  delete dmm->blockids;
  delete dmm->blocknames;
  if (dmm->sides)   delete dmm->sides;
  delete dmm->sideids;
  delete dmm->sidenames;
  if (dmm->unsides) delete dmm->unsides;
  delete dmm->unsideids;
  delete dmm->unsidenames;
  if (dmm->contacts)   delete dmm->contacts;
  delete dmm->contactnames;
  delete dmm->contact_displaced;
  if (dmm->uncontacts) delete dmm->uncontacts;
  delete dmm->uncontactnames;
  delete dmm->uncontact_displaced;
  if(dmm->splits) {
    for (std::map<std::string, DM_Moose::SplitInfo>::iterator sit = dmm->splits->begin(); sit != dmm->splits->end(); ++sit) {
      ierr = DMDestroy(&(sit->second.dm));CHKERRQ(ierr);
      ierr = ISDestroy(&(sit->second.rembedding));CHKERRQ(ierr);
    }
    delete dmm->splits;
  }
  if(dmm->splitlocs) delete dmm->splitlocs;
  ierr = ISDestroy(&dmm->embedding); CHKERRQ(ierr);
  ierr = PetscFree(dm->data); CHKERRQ(ierr);
  PetscFunctionReturn(0);

}


#undef __FUNCT__
#define __FUNCT__ "DMCreateMoose"
PetscErrorCode  DMCreateMoose(MPI_Comm comm, NonlinearSystem& nl, DM *dm)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DMCreate(comm, dm);CHKERRQ(ierr);
  ierr = DMSetType(*dm, DMMOOSE);CHKERRQ(ierr);
  ierr = DMMooseSetNonlinearSystem(*dm,nl);CHKERRQ(ierr);
  PetscFunctionReturn(0);

}

EXTERN_C_BEGIN
#undef __FUNCT__
#define __FUNCT__ "DMCreate_Moose"
PetscErrorCode  DMCreate_Moose(DM dm)
{
  PetscErrorCode ierr;
  DM_Moose     *dmm;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  ierr = PetscNewLog(dm,DM_Moose,&dmm);CHKERRQ(ierr);
  dm->data = dmm;

  dmm->varids        = new(std::map<std::string, unsigned int>);
  dmm->blockids      = new(std::map<std::string, unsigned int>);
  dmm->varnames      = new(std::map<unsigned int, std::string>);
  dmm->blocknames    = new(std::map<unsigned int, std::string>);
  dmm->sideids       = new(std::map<std::string, BoundaryID>);
  dmm->sidenames     = new(std::map<BoundaryID,std::string>);
  dmm->unsideids     = new(std::map<std::string, BoundaryID>);
  dmm->unsidenames   = new(std::map<BoundaryID,std::string>);
  dmm->contactnames  = new(std::map<DM_Moose::ContactID,DM_Moose::ContactName>);
  dmm->uncontactnames= new(std::map<DM_Moose::ContactID,DM_Moose::ContactName>);
  dmm->contact_displaced= new(std::map<DM_Moose::ContactName,PetscBool>);
  dmm->uncontact_displaced= new(std::map<DM_Moose::ContactName,PetscBool>);

  dmm->splits = new(std::map<std::string,DM_Moose::SplitInfo>);

  dmm->print_embedding = PETSC_FALSE;

  dm->ops->createglobalvector = DMCreateGlobalVector_Moose;
  dm->ops->createlocalvector  = 0; // DMCreateLocalVector_Moose;
  dm->ops->getcoloring        = 0; // DMGetColoring_Moose;
  dm->ops->creatematrix       = DMCreateMatrix_Moose;
  dm->ops->createinterpolation= 0; // DMCreateInterpolation_Moose;

  dm->ops->refine             = 0; // DMRefine_Moose;
  dm->ops->coarsen            = 0; // DMCoarsen_Moose;
  dm->ops->getinjection       = 0; // DMGetInjection_Moose;
  dm->ops->getaggregates      = 0; // DMGetAggregates_Moose;

#if PETSC_VERSION_LE(3,3,0) && PETSC_VERSION_RELEASE
  dm->ops->createfielddecompositiondm  = DMCreateFieldDecompositionDM_Moose;
  dm->ops->createdomaindecompositiondm = DMCreateDomainDecompositionDM_Moose;
#endif
  dm->ops->createfielddecomposition    = DMCreateFieldDecomposition_Moose;
  dm->ops->createdomaindecomposition   = DMCreateDomainDecomposition_Moose;

  dm->ops->destroy            = DMDestroy_Moose;
  dm->ops->view               = DMView_Moose;
  dm->ops->setfromoptions     = DMSetFromOptions_Moose;
  dm->ops->setup              = DMSetUp_Moose;
  PetscFunctionReturn(0);

}
EXTERN_C_END


#endif // #if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
