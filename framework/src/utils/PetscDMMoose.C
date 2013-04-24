#include "libmesh/petsc_macro.h"
// This only works with petsc-3.3 and above.

#if !PETSC_VERSION_LESS_THAN(3,3,0)

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

struct DM_Moose
{
  NonlinearSystem*                      nl;
  std::set<std::string>                *vars;
  std::map<std::string, unsigned int>  *varids;
  std::map<unsigned int, std::string>  *varnames;
  std::set<std::string>                *blocks;
  std::map<std::string, unsigned int>  *blockids;
  std::map<unsigned int, std::string>  *blocknames;
  std::set<std::string>                *sides;
  std::map<BoundaryID, std::string>    *sidenames;
  std::map<std::string, BoundaryID>    *sideids;
  struct SplitNames {
    std::set<std::string> varnames;
    std::set<std::string> blocknames;
    std::set<std::string> sidenames;
  };
  std::map<std::string, SplitNames > *splits;
  struct SplitIDs {
    std::set<unsigned int> varids;
    std::set<unsigned int> blockids;
    std::set<BoundaryID>   sideids;
  };
  std::map<std::string, SplitIDs > *field_decomposition;
  std::map<std::string,std::set<std::string> > *subdomains;
  std::map<std::string,std::set<unsigned int> > *domain_decomposition;
  IS                                    embedding;
};

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
PetscErrorCode DMMooseSetSplitNames(DM dm, const std::set<std::string>& splitnames)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  PetscBool ismoose;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);

  if (dmm->field_decomposition) {
    delete dmm->field_decomposition;
    dmm->field_decomposition = PETSC_NULL;
  }
  if (dmm->splits) {
    delete dmm->splits;
    dmm->splits = PETSC_NULL;
  }
  dmm->splits = new std::map<std::string, DM_Moose::SplitNames >();
  for (std::set<std::string>::const_iterator it = splitnames.begin(); it != splitnames.end(); ++it) {
    (*dmm->splits)[*it] = DM_Moose::SplitNames();
  }
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetSplitNames"
PetscErrorCode DMMooseGetSplitNames(DM dm, std::set<std::string>& splitnames)
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
  if (dmm->field_decomposition && dmm->field_decomposition->size()) {
    for (std::map<std::string, DM_Moose::SplitIDs >::const_iterator sit = dmm->field_decomposition->begin(); sit != dmm->field_decomposition->end(); ++sit) {
      std::string sname = sit->first;
      splitnames.insert(sname);
    }
  }
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseSetSplitVars"
PetscErrorCode DMMooseSetSplitVars(DM dm, const std::string& splitname, const std::set<std::string>& splitvars)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  PetscBool ismoose;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);
  if (dm->setupcalled) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "DM already set up");
  if (!dmm->splits->count(splitname)) SETERRQ1(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown split name %s", splitname.c_str());
  (*dmm->splits)[splitname].varnames = splitvars;
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetSplitVars"
PetscErrorCode DMMooseGetSplitVars(DM dm, const std::string& splitname, std::set<std::string>& splitvars)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  PetscBool ismoose;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);
  if (!dm->setupcalled) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "DM not set up");
  if (!dmm->field_decomposition || !dmm->field_decomposition->size() || !dmm->field_decomposition->count(splitname)) {
    SETERRQ1(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown split name %s", splitname.c_str());
  }
  DM_Moose::SplitIDs& splitids = (*dmm->field_decomposition)[splitname];
  splitvars.clear();
  for (std::set<unsigned int>::const_iterator idit = splitids.varids.begin(); idit != splitids.varids.end(); ++idit) {
    std::map<unsigned int, std::string>::const_iterator nameit = dmm->varnames->find(*idit);
    if (nameit == dmm->varnames->end()) SETERRQ2(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown varid %D in split %s", *idit, splitname.c_str());
    splitvars.insert(nameit->second);
  }
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseSetSplitBlocks"
PetscErrorCode DMMooseSetSplitBlocks(DM dm, const std::string& splitname, const std::set<std::string>& splitblocks)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  PetscBool ismoose;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);
  if (dm->setupcalled) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "DM already set up");
  if (!dmm->splits->count(splitname)) SETERRQ1(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown split name %s", splitname.c_str());
  (*dmm->splits)[splitname].blocknames = splitblocks;
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetSplitBlocks"
PetscErrorCode DMMooseGetSplitBlocks(DM dm, const std::string& splitname, std::set<std::string>& splitblocks)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  PetscBool ismoose;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);
  if (!dm->setupcalled) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "DM not set up");
  if (!dmm->field_decomposition || !dmm->field_decomposition->size() || !dmm->field_decomposition->count(splitname)) {
    SETERRQ1(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown split name %s", splitname.c_str());
  }
  DM_Moose::SplitIDs& splitids = (*dmm->field_decomposition)[splitname];
  splitblocks.clear();
  for (std::set<unsigned int>::const_iterator idit = splitids.blockids.begin(); idit != splitids.blockids.end(); ++idit) {
    std::map<unsigned int, std::string>::const_iterator nameit = dmm->blocknames->find(*idit);
    if (nameit == dmm->blocknames->end()) SETERRQ2(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown blockid %D in split %s", *idit, splitname.c_str());
    splitblocks.insert(nameit->second);
  }
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseSetSplitSides"
PetscErrorCode DMMooseSetSplitSides(DM dm, const std::string& splitname, const std::set<std::string>& splitsides)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  PetscBool ismoose;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);
  if (dm->setupcalled) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "DM already set up");
  if (!dmm->splits->count(splitname)) SETERRQ1(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown split name %s", splitname.c_str());
  (*dmm->splits)[splitname].sidenames = splitsides;
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetSplitSides"
PetscErrorCode DMMooseGetSplitSides(DM dm, const std::string& splitname, std::set<std::string>& splitsides)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  PetscBool ismoose;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);
  if (!dm->setupcalled) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONGSTATE, "DM not set up");
  if (!dmm->field_decomposition || !dmm->field_decomposition->size() || !dmm->field_decomposition->count(splitname)) {
    SETERRQ1(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown split name %s", splitname.c_str());
  }
  DM_Moose::SplitIDs& splitids = (*dmm->field_decomposition)[splitname];
  splitsides.clear();
  for (std::set<BoundaryID>::const_iterator idit = splitids.sideids.begin(); idit != splitids.sideids.end(); ++idit) {
    std::map<BoundaryID, std::string>::const_iterator nameit = dmm->sidenames->find(*idit);
    if (nameit == dmm->sidenames->end()) SETERRQ2(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown sideid %D in split %s", *idit, splitname.c_str());
    splitsides.insert(nameit->second);
  }
  PetscFunctionReturn(0);
}


#undef  __FUNCT__
#define __FUNCT__ "DMMooseSetDomainDecomposition"
PetscErrorCode DMMooseSetDomainDecomposition(DM dm, const std::map<std::string, std::set<std::string> >& subdomains)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  PetscBool ismoose;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);

  if (dmm->domain_decomposition) {
    delete dmm->domain_decomposition;
    dmm->domain_decomposition = PETSC_NULL;
  }
  dmm->subdomains = new std::map<std::string, std::set<std::string> >(subdomains);
  PetscFunctionReturn(0);

}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetDomainDecomposition"
PetscErrorCode DMMooseGetDomainDecomposition(DM dm, std::map<std::string, std::set<std::string> >& subdomains)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  PetscBool ismoose;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);

  subdomains.clear();
  if (!dmm->domain_decomposition || !dmm->domain_decomposition->size()) PetscFunctionReturn(0);
  for (std::map<std::string, std::set<unsigned int> >::const_iterator sit = dmm->domain_decomposition->begin(); sit != dmm->domain_decomposition->end(); ++sit) {
    std::string sname = sit->first;
    subdomains[sname] = std::set<std::string>();
    for (std::set<unsigned int>::const_iterator it = (*dmm->domain_decomposition)[sname].begin(); it != (*dmm->domain_decomposition)[sname].end(); ++it) {
      subdomains[sname].insert((*dmm->blocknames)[*it]);
    }
  }
  PetscFunctionReturn(0);

}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseSetEmbedding_Private"
static PetscErrorCode DMMooseSetEmbedding_Private(DM dm, IS embedding, const std::string& subname, DM ddm)
{
  DM_Moose       *ddmm=(DM_Moose*)ddm->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscObjectReference((PetscObject)embedding);CHKERRQ(ierr);
  ddmm->embedding = embedding;
  ierr = PetscObjectSetOptionsPrefix((PetscObject)ddm,((PetscObject)dm)->prefix);CHKERRQ(ierr);
  std::string suffix = std::string("fieldsplit_")+subname+"_";
  ierr = PetscObjectAppendOptionsPrefix((PetscObject)ddm,suffix.c_str());CHKERRQ(ierr);
  PetscFunctionReturn(0);

}

#undef __FUNCT__
#define __FUNCT__ "DMCreateFieldDecomposition_Moose"
static PetscErrorCode  DMCreateFieldDecomposition_Moose(DM dm, PetscInt *len, char ***namelist, IS **islist, DM **dmlist)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose *)(dm->data);
  IS             emb;

  PetscFunctionBegin;
  /* Only called after DMSetUp(). */
  if(!dmm->field_decomposition) PetscFunctionReturn(0);
  *len = dmm->field_decomposition->size();
  if(namelist) {ierr = PetscMalloc(*len*sizeof(char*),namelist);CHKERRQ(ierr);}
  if(islist)   {ierr = PetscMalloc(*len*sizeof(IS),islist);CHKERRQ(ierr);}
  if(dmlist)   {ierr = PetscMalloc(*len*sizeof(DM),dmlist);CHKERRQ(ierr);}
  DofMap& dofmap = dmm->nl->sys().get_dof_map();
  unsigned int d = 0;
  for (std::map<std::string, DM_Moose::SplitIDs >::const_iterator dit = dmm->field_decomposition->begin(); dit != dmm->field_decomposition->end(); ++dit,++d) {
    std::string                          dname = dit->first;
    std::set<std::string>                dvars;
    std::set<std::string>                dblocks;
    std::set<std::string>                dsides;
    std::set<unsigned int>               dindices;
    for(std::set<unsigned int>::const_iterator dvit = dit->second.varids.begin(); dvit != dit->second.varids.end(); ++dvit){
      unsigned int v = *dvit;
      std::string vname = (*dmm->varnames)[v];
      dvars.insert(vname);
      if(!islist) continue;
      /* Iterate only over this split's blocks. */
      for(std::set<unsigned int>::const_iterator bit = dit->second.blockids.begin(); bit != dit->second.blockids.end(); ++bit) {
	unsigned int b = *bit;
	std::string bname = (*dmm->blocknames)[b];
	dblocks.insert(bname);
	MeshBase::const_element_iterator el     = dmm->nl->sys().get_mesh().active_local_subdomain_elements_begin(b);
	MeshBase::const_element_iterator end_el = dmm->nl->sys().get_mesh().active_local_subdomain_elements_end(b);
	for ( ; el != end_el; ++el) {
	  const Elem* elem = *el;
	  //unsigned int e_subdomain = elem->subdomain_id();
	  std::vector<unsigned int> evindices;
	  // Get the degree of freedom indices for the given variable off the current element.
	  dofmap.dof_indices(elem, evindices, v);
	  for(unsigned int i = 0; i < evindices.size(); ++i) {
	    unsigned int dof = evindices[i];
	    if(dof >= dofmap.first_dof() && dof < dofmap.end_dof()) /* might want to use variable_first/last_local_dof instead */
	      dindices.insert(dof);
	  }
	}
      }
      /* Iterate over all sides and pick out only the ones that belong to this split. */
      if (dit->second.sideids.size()) {
	std::vector<unsigned int> snodes;
	std::vector<boundary_id_type> sides;
	dmm->nl->sys().get_mesh().boundary_info->build_node_list(snodes, sides);
	// FIXME: make an array of (snode,side) pairs, sort on side and use std::lower_bound from <algorithm>
	for (unsigned int i = 0; i < sides.size(); ++i) {
	  boundary_id_type s = sides[i];
	  if (!dit->second.sideids.count(s)) continue;
	  // Get the degree of freedom indices for all of this split's variables off the current node.
	  for (std::set<unsigned int>::const_iterator vit = dit->second.varids.begin(); vit != dit->second.varids.end(); ++vit) {
	    const Node& node = dmm->nl->sys().get_mesh().node(snodes[i]);
	    dof_id_type dof = node.dof_number(dmm->nl->sys().number(),*vit,0);
	    if(dof >= dofmap.first_dof() && dof < dofmap.end_dof()) { /* might want to use variable_first/last_local_dof instead */
	      dindices.insert(dof);
	    }
	  }
	}
      }
    }
   if(namelist) {
      ierr = PetscStrallocpy(dname.c_str(),(*namelist)+d);CHKERRQ(ierr);
    }
    if(islist) {
      IS dis;
      PetscInt *darray;
      ierr = PetscMalloc(sizeof(PetscInt)*dindices.size(),&darray);CHKERRQ(ierr);
      unsigned int i = 0;
      for(std::set<unsigned int>::const_iterator it = dindices.begin(); it != dindices.end(); ++it) {
	darray[i] = *it;
	++i;
      }
      ierr = ISCreateGeneral(((PetscObject)dm)->comm, dindices.size(),darray, PETSC_OWN_POINTER, &dis); CHKERRQ(ierr);
      if(dmm->embedding) {
	/* Create a relative embedding into the parent's index space. */
#if PETSC_VERSION_LE(3,3,0) && PETSC_VERSION_RELEASE
	ierr = ISMapFactorRight(dis,dmm->embedding, PETSC_TRUE, &emb); CHKERRQ(ierr);
#else
	ierr = ISEmbed(dis,dmm->embedding, PETSC_TRUE, &emb); CHKERRQ(ierr);
#endif
	PetscInt elen, dlen;
	ierr = ISGetLocalSize(emb, &elen); CHKERRQ(ierr);
	ierr = ISGetLocalSize(dis, &dlen); CHKERRQ(ierr);
	if(elen != dlen) SETERRQ1(((PetscObject)dm)->comm, PETSC_ERR_PLIB, "Failed to embed subdomain %D", d);
	ierr = ISDestroy(&dis); CHKERRQ(ierr);
	dis = emb;
      }
      else {
	emb = dis;
      }
      (*islist)[d] = dis;
    }
    if (dmlist) {
      /* Translate side ids to side names. */
      for (std::set<boundary_id_type>::const_iterator sit = dit->second.sideids.begin(); sit != dit->second.sideids.end(); ++sit) {
	boundary_id_type s = *sit;
	std::string sname = (*dmm->sidenames)[s];
	dsides.insert(sname);
      }
      DM ddm;
      ierr = DMCreateMoose(((PetscObject)dm)->comm, *dmm->nl, &ddm);CHKERRQ(ierr);
      ierr = DMMooseSetVariables(ddm,dvars);CHKERRQ(ierr);
      ierr = DMMooseSetBlocks(ddm,dblocks);CHKERRQ(ierr);
      ierr = DMMooseSetSides(ddm,dsides);CHKERRQ(ierr);
      ierr = DMMooseSetEmbedding_Private(dm,emb,dname,ddm);CHKERRQ(ierr);
      ierr = DMSetFromOptions(ddm);CHKERRQ(ierr);
      ierr = DMSetUp(ddm);CHKERRQ(ierr);
      (*dmlist)[d] = ddm;
    }
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMCreateDomainDecomposition_Moose"
static PetscErrorCode  DMCreateDomainDecomposition_Moose(DM dm, PetscInt *len, char ***namelist, IS **innerislist, IS **outerislist, DM **dmlist)
{
  PetscErrorCode ierr;
  DM_Moose     *dmm = (DM_Moose *)(dm->data);
  IS emb;

  PetscFunctionBegin;
  /* Only called after DMSetUp(). */
  if(!dmm->domain_decomposition) PetscFunctionReturn(0);
  *len = dmm->domain_decomposition->size();
  if(namelist)      {ierr = PetscMalloc(*len*sizeof(char*), namelist);  CHKERRQ(ierr);}
  if(innerislist)   {ierr = PetscMalloc(*len*sizeof(IS),    innerislist);    CHKERRQ(ierr);}
  if(outerislist)   *outerislist = PETSC_NULL; /* FIX: allow mesh-based overlap. */
  if(dmlist)        {ierr = PetscMalloc(*len*sizeof(DM),    dmlist);    CHKERRQ(ierr);}
  unsigned int d = 0;
  for (std::map<std::string, std::set<unsigned int> >::const_iterator dit= dmm->domain_decomposition->begin(); dit != dmm->domain_decomposition->end(); ++dit,++d){
    std::string                          dname = dit->first;
    std::set<std::string>                dvars;
    std::set<std::string>                dblocks;
    std::set<unsigned int>               dindices;
    for(std::set<unsigned int>::const_iterator bit = dit->second.begin(); bit != dit->second.end(); ++bit){
      unsigned int b = *bit;
      dblocks.insert((*dmm->blocknames)[b]);
      if(!innerislist) continue;
      MeshBase::const_element_iterator       el     = dmm->nl->sys().get_mesh().active_local_subdomain_elements_begin(b);
      const MeshBase::const_element_iterator end_el = dmm->nl->sys().get_mesh().active_local_subdomain_elements_end(b);
      for ( ; el != end_el; ++el) {
	const Elem* elem = *el;
	std::vector<unsigned int> evindices;
	/* Iterate only over this DM's variables. */
	for(std::map<std::string, unsigned int>::const_iterator vit = dmm->varids->begin(); vit != dmm->varids->end(); ++vit) {
	  unsigned int v = vit->second;
	  dvars.insert(vit->first);
	  // Get the degree of freedom indices for the given variable off the current element.
	  dmm->nl->sys().get_dof_map().dof_indices(elem, evindices, v);
	  for(unsigned int i = 0; i < evindices.size(); ++i) {
	    unsigned int dof = evindices[i];
	    if(dof >= dmm->nl->sys().get_dof_map().first_dof() && dof < dmm->nl->sys().get_dof_map().end_dof()) /* might want to use variable_first/last_local_dof instead */
	      dindices.insert(dof);
	  }
	}
      }
    }
    if(namelist) {
      ierr = PetscStrallocpy(dname.c_str(),(*namelist)+d);            CHKERRQ(ierr);
    }
    if(innerislist) {
      PetscInt *darray;
      IS dis;
      ierr = PetscMalloc(sizeof(PetscInt)*dindices.size(), &darray); CHKERRQ(ierr);
      unsigned int i = 0;
      for(std::set<unsigned int>::const_iterator it = dindices.begin(); it != dindices.end(); ++it) {
	darray[i] = *it;
	++i;
      }
      ierr = ISCreateGeneral(((PetscObject)dm)->comm, dindices.size(),darray, PETSC_OWN_POINTER, &dis); CHKERRQ(ierr);
      if(dmm->embedding) {
	/* Create a relative embedding into the parent's index space. */
#if PETSC_VERSION_LE(3,3,0) && PETSC_VERSION_RELEASE
	ierr = ISMapFactorRight(dis,dmm->embedding, PETSC_TRUE, &emb); CHKERRQ(ierr);
#else
	ierr = ISEmbed(dis,dmm->embedding, PETSC_TRUE, &emb); CHKERRQ(ierr);
#endif
	PetscInt elen, dlen;
	ierr = ISGetLocalSize(emb, &elen); CHKERRQ(ierr);
	ierr = ISGetLocalSize(dis, &dlen);  CHKERRQ(ierr);
	if(elen != dlen) SETERRQ1(((PetscObject)dm)->comm, PETSC_ERR_PLIB, "Failed to embed field %D", d);
	ierr = ISDestroy(&dis); CHKERRQ(ierr);
	dis = emb;
      }
      else {
	emb = dis;
      }
      if(innerislist) {
	ierr = PetscObjectReference((PetscObject)dis); CHKERRQ(ierr);
	(*innerislist)[d] = dis;
      }
      ierr = ISDestroy(&dis); CHKERRQ(ierr);
    }
    if(dmlist) {
      DM ddm;
      ierr = DMCreateMoose(((PetscObject)dm)->comm, *dmm->nl, &ddm);CHKERRQ(ierr);
      ierr = DMMooseSetVariables(ddm,dvars);CHKERRQ(ierr);
      ierr = DMMooseSetBlocks(ddm,dblocks);CHKERRQ(ierr);
      ierr = DMMooseSetEmbedding_Private(dm,emb,dname,ddm);CHKERRQ(ierr);
      ierr = DMSetFromOptions(ddm);           CHKERRQ(ierr);
      ierr = DMSetUp(ddm);CHKERRQ(ierr);
      (*dmlist)[d] = ddm;
    }
  }
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
static PetscErrorCode DMCreateMatrix_Moose(DM dm, const MatType, Mat *A)
{
  PetscErrorCode ierr;
  DM_Moose       *dmm = (DM_Moose *)(dm->data);
  PetscBool      ismoose;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);CHKERRQ(ierr);
  if (!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "DM of type %s, not of type %s", ((PetscObject)dm)->type, DMMOOSE);
  if (!dmm->nl) SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_ARG_WRONGSTATE, "No Moose system set for DM_Moose");
  *A = (dynamic_cast<PetscMatrix<Number>*>(dmm->nl->sys().matrix))->mat();
  ierr = PetscObjectReference((PetscObject)(*A));CHKERRQ(ierr);
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
    ierr = PetscViewerASCIIPrintf(viewer, "\n"); CHKERRQ(ierr);
    ierr = PetscViewerASCIIPrintf(viewer, "blocks:", name, prefix); CHKERRQ(ierr);
    std::map<std::string,unsigned int>::iterator bit = dmm->blockids->begin();
    std::map<std::string,unsigned int>::const_iterator bend = dmm->blockids->end();
    for(; bit != bend; ++bit) {
      ierr = PetscViewerASCIIPrintf(viewer, "(%s,%D) ", bit->first.c_str(), bit->second); CHKERRQ(ierr);
    }
    ierr = PetscViewerASCIIPrintf(viewer, "\n"); CHKERRQ(ierr);
    if (dmm->sideids->size()) {
      ierr = PetscViewerASCIIPrintf(viewer, "sides:", name, prefix); CHKERRQ(ierr);
      std::map<std::string,BoundaryID>::iterator sit = dmm->sideids->begin();
      std::map<std::string,BoundaryID>::const_iterator send = dmm->sideids->end();
      for (; sit != send; ++sit) {
	ierr = PetscViewerASCIIPrintf(viewer, "(%s,%D) ", sit->first.c_str(), sit->second);CHKERRQ(ierr);
      }
      ierr = PetscViewerASCIIPrintf(viewer, "\n");CHKERRQ(ierr);
    }
    if (dmm->domain_decomposition && dmm->domain_decomposition->size()) {
      ierr = PetscViewerASCIIPrintf(viewer, "Domain decomposition: blocks: ");CHKERRQ(ierr);
      /* FIX: decompositions might have different sizes and components on different ranks. */
      for (std::map<std::string, std::set<unsigned int> >::const_iterator dit = dmm->domain_decomposition->begin(); dit != dmm->domain_decomposition->end(); ++dit) {
	ierr = PetscViewerASCIIPrintf(viewer, "%s: ", dit->first.c_str());CHKERRQ(ierr);
	for(std::set<unsigned int>::const_iterator it = dit->second.begin(); it != dit->second.end(); ++it) {
	  if(it != dit->second.begin()) {
	    ierr = PetscViewerASCIIPrintf(viewer, ",");CHKERRQ(ierr);
	  }
	  ierr = PetscViewerASCIIPrintf(viewer,"(%D,%s)",*it,(*dmm->blocknames)[*it].c_str());CHKERRQ(ierr);
	}
	ierr = PetscViewerASCIIPrintf(viewer, ";");CHKERRQ(ierr);
      }
      ierr = PetscViewerASCIIPrintf(viewer, "\n");CHKERRQ(ierr);
    }
    if (dmm->field_decomposition && dmm->field_decomposition->size()) {
      ierr = PetscViewerASCIIPrintf(viewer, "Field decomposition:");CHKERRQ(ierr);
      /* FIX: decompositions might have different sizes and components on different ranks. */
      for (std::map<std::string, DM_Moose::SplitIDs>::const_iterator dit = dmm->field_decomposition->begin(); dit != dmm->field_decomposition->end(); ++dit) {
	ierr = PetscViewerASCIIPrintf(viewer, " %s:[", dit->first.c_str());CHKERRQ(ierr);
	ierr = PetscViewerASCIIPrintf(viewer, "vars:");CHKERRQ(ierr);
	for (std::set<unsigned int>::const_iterator vit = dit->second.varids.begin(); vit != dit->second.varids.end(); ++vit) {
	  if (vit != dit->second.varids.begin()) {
	    ierr = PetscViewerASCIIPrintf(viewer, ",");CHKERRQ(ierr);
	  }
	  ierr = PetscViewerASCIIPrintf(viewer,"%s",(*dmm->varnames)[*vit].c_str());CHKERRQ(ierr);
	}
	ierr = PetscViewerASCIIPrintf(viewer, "; blocks:");CHKERRQ(ierr);
	for (std::set<unsigned int>::const_iterator bit = dit->second.blockids.begin(); bit != dit->second.blockids.end(); ++bit) {
	  if (bit != dit->second.blockids.begin()) {
	    ierr = PetscViewerASCIIPrintf(viewer, ",");CHKERRQ(ierr);
	  }
	  ierr = PetscViewerASCIIPrintf(viewer,"%s",(*dmm->blocknames)[*bit].c_str());CHKERRQ(ierr);
	}
	if (dit->second.sideids.size() ) {
	  ierr = PetscViewerASCIIPrintf(viewer, "; sides:");CHKERRQ(ierr);
	  for (std::set<BoundaryID>::const_iterator sit = dit->second.sideids.begin(); sit != dit->second.sideids.end(); ++sit) {
	    if (sit != dit->second.sideids.begin()) {
	      ierr = PetscViewerASCIIPrintf(viewer, ",");CHKERRQ(ierr);
	    }
	    ierr = PetscViewerASCIIPrintf(viewer,"%s",(*dmm->sidenames)[*sit].c_str());CHKERRQ(ierr);
	  }
	}
	ierr = PetscViewerASCIIPrintf(viewer, " ]");CHKERRQ(ierr);
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
#define __FUNCT__ "DMSetUp_Moose_Private"
static PetscErrorCode  DMSetUp_Moose_Private(DM dm)
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
  dmm->varids->clear();
  dmm->varnames->clear();
  // FIXME: would be nice to invert this nested loop structure so we could iterate over the potentially smaller dmm->vars,
  // but checking against dofmap.variable would still require a linear search, hence, no win.  Would be nice to endow dofmap.variable
  // with a fast search capability.
  for(unsigned int v = 0; v < dofmap.n_variables(); ++v) {
    std::string vname = dofmap.variable(v).name();
    if (dmm->vars && dmm->vars->find(vname) == dmm->vars->end()) continue;
    dmm->varids->insert(std::pair<std::string,unsigned int>(vname,v));
    dmm->varnames->insert(std::pair<unsigned int,std::string>(v,vname));
  }
  if (dmm->vars) {
    delete dmm->vars;
    dmm->vars = PETSC_NULL;
  }

  const MeshBase& mesh = dmm->nl->sys().get_mesh();
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
    if (dmm->blocks && dmm->blocks->find(bname) == dmm->blocks->end()) continue;
    dmm->blockids->insert(std::pair<std::string,unsigned int>(bname,bid));
    dmm->blocknames->insert(std::pair<unsigned int,std::string>(bid,bname));
  }
  if (dmm->blocks) {
    delete dmm->blocks;
    dmm->blocks = PETSC_NULL;
  }

  if (dmm->sides) {
    const std::set<boundary_id_type>& sides = mesh.boundary_info->get_boundary_ids();
    for (std::set<boundary_id_type>::const_iterator sit = sides.begin(); sit != sides.end(); ++sit) {
      boundary_id_type sid = *sit;
      std::string sname = mesh.boundary_info->sideset_name(sid);
      if (dmm->sides->count(sname)) {
	dmm->sidenames->insert(std::pair<boundary_id_type,std::string>(sid,sname));
	dmm->sideids->insert(std::pair<std::string,boundary_id_type>(sname,sid));
      }
    }
    delete dmm->sides;
    dmm->sides = PETSC_NULL;
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
  ierr = PetscObjectSetName((PetscObject)dm,name.c_str());CHKERRQ(ierr);
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
  /*
     Set up decompositions.
   */
  if (dmm->splits) {
    delete dmm->field_decomposition;
    dmm->field_decomposition = new std::map<std::string, DM_Moose::SplitIDs >();
    unsigned int i = 0;
    for (std::map<std::string, DM_Moose::SplitNames >::const_iterator spit = dmm->splits->begin(); spit != dmm->splits->end(); ++spit, ++i) {
      std::string spname = spit->first;
      if (dmm->field_decomposition->count(spname)) {
	SETERRQ1(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Duplicate split name: %s", spname.c_str());
      }
      (*dmm->field_decomposition)[spname] = DM_Moose::SplitIDs();
      unsigned int j;
      j = 0;
      for (std::set<std::string>::const_iterator it = spit->second.varnames.begin(); it != spit->second.varnames.end(); ++it, ++j) {
	std::map<std::string, unsigned int>::const_iterator vit = dmm->varids->find(*it);
	if (vit == dmm->varids->end()) {
	  SETERRQ4(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown %D-th variable %s in %D-th split %s", i, it->c_str(), j, spname.c_str());
	} else {
	  (*dmm->field_decomposition)[spname].varids.insert(vit->second);
	}
      }
      // Default behavior: empty spit->second.blocknames implies using ALL of DM's blocks.
      // Validate the split blocks first.
      j = 0;
      for (std::set<std::string>::const_iterator it = spit->second.blocknames.begin(); it != spit->second.blocknames.end(); ++it, ++j) {
	std::map<std::string, unsigned int>::const_iterator bit = dmm->blockids->find(*it);
	if (bit == dmm->blockids->end()) {
	  SETERRQ4(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown %D-th block %s in %D-th split %s", i, it->c_str(), j, spname.c_str());
	}
      }
      for (std::map<std::string, unsigned int>::const_iterator bit = dmm->blockids->begin(); bit != dmm->blockids->end(); ++bit) {
	if (spit->second.blocknames.size() && !spit->second.blocknames.count(bit->first)) continue;
	(*dmm->field_decomposition)[spname].blockids.insert(bit->second);
      }
      j = 0;
      for (std::set<std::string>::const_iterator it = spit->second.sidenames.begin(); it != spit->second.sidenames.end(); ++it, ++j) {
	std::map<std::string, BoundaryID>::const_iterator sit = dmm->sideids->find(*it);
	if (sit == dmm->sideids->end()) {
	  SETERRQ4(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown %D-th block %s in %D-th split %s", i, it->c_str(), j, spname.c_str());
	} else {
	  (*dmm->field_decomposition)[spname].sideids.insert(sit->second);
	}
      }
    }
    delete dmm->splits;
    dmm->splits = PETSC_NULL;
  }
  if (dmm->subdomains) {
    dmm->domain_decomposition = new std::map<std::string, std::set<unsigned int> >();
    unsigned int i = 0;
    for (std::map<std::string, std::set<std::string> >::const_iterator sit = dmm->subdomains->begin(); sit != dmm->subdomains->end(); ++sit, ++i) {
      std::string sname = sit->first;
      if (dmm->domain_decomposition->count(sname)) SETERRQ1(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Duplicate subdomain name: %s", sname.c_str());
      (*dmm->domain_decomposition)[sname] = std::set<unsigned int>();
      unsigned int j = 0;
      for (std::set<std::string>::iterator it = sit->second.begin(); it != sit->second.end(); ++it, ++j) {
	std::map<std::string, unsigned int>::const_iterator bit = dmm->blockids->find(*it);
	if (bit == dmm->blockids->end()) {
	  SETERRQ3(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown %D-th mesh block %s in split %D", i, it->c_str(), j);
	} else {
	  (*dmm->domain_decomposition)[sname].insert(bit->second);
	}
      }
    }
    delete dmm->subdomains;
    dmm->subdomains = PETSC_NULL;
  }

  /*
     Do not evaluate function, Jacobian or bounds for an embedded DM -- the subproblem might not have enough information for that.
  */
  if(!dmm->embedding) {
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
  PetscInt nsplits = 0;
  /* Insert the usage of -dm_moose_fieldsplit_names and -dm_moose_fieldsplit_<splitname> into this help message, since the following if-clause might never fire, if -help is requested. */
  const char* fdhelp = "Number of named fieldsplits defined by the DM.\n\
                \tNames of fieldsplits are defined by -dm_moose_fieldsplit_names <splitname1> <splitname2> ...\n\
                \tEach split defined by the variables, blocks and sides that belong to it as follows: -dm_moose_fieldsplit_<splitname>_vars <var1> <var2> ...\n\
                \tLikewise for blocks and sides.  Empty block list implies 'all blocks', while an empty side list implies 'no sides'\n\
                \tIf no -dm_moose_fieldsplit_names is given, <splitname> is 0, 1, ...";
  ierr = PetscOptionsInt("-dm_moose_nfieldsplits", fdhelp, "DMMooseSetSplitNames", nsplits, &nsplits, NULL);CHKERRQ(ierr);
  if (nsplits) {
    PetscInt nnsplits = nsplits;
    std::set<std::string> split_names;
    char** splitnames;
    ierr = PetscMalloc(nsplits*sizeof(char*),&splitnames);CHKERRQ(ierr);
    ierr = PetscOptionsStringArray("-dm_moose_fieldsplit_names", "Names of fieldsplits defined by the DM", "DMMooseSetSplitNames", splitnames, &nnsplits, NULL);CHKERRQ(ierr);
    if (!nnsplits) {
      for (PetscInt i = 0; i < nsplits; ++i) {
	std::ostringstream s;
	s << i;
	split_names.insert(s.str());
      }
    } else if (nsplits != nnsplits) {
      SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_SIZ, "Expected %D fieldsplit names, got %D instead", nsplits, nnsplits);
    } else {
      for (PetscInt i = 0; i < nsplits; ++i) {
	split_names.insert(std::string(splitnames[i]));
      }
    }
    ierr = PetscFree(splitnames);CHKERRQ(ierr);
    ierr = DMMooseSetSplitNames(dm, split_names);CHKERRQ(ierr);
    for (std::set<std::string>::const_iterator sit = split_names.begin(); sit != split_names.end(); ++sit) {
      std::string spname = *sit;
      {
	PetscInt maxvars = dmm->nl->sys().get_dof_map().n_variables();
	char** splitvars;
	std::set<std::string> split_vars;
	PetscInt nvars = maxvars;
	ierr = PetscMalloc(maxvars*sizeof(char*),&splitvars);CHKERRQ(ierr);
	std::string opt = "-dm_moose_fieldsplit_"+*sit+"_vars";
	std::string help = "Variables in fieldsplit "+*sit;
	ierr = PetscOptionsStringArray(opt.c_str(),help.c_str(),"DMMooseSetSplitVars",splitvars,&nvars,PETSC_NULL);CHKERRQ(ierr);
	for (PetscInt i = 0; i < nvars; ++i) {
	  split_vars.insert(std::string(splitvars[i]));
	}
	ierr = PetscFree(splitvars);CHKERRQ(ierr);
	ierr = DMMooseSetSplitVars(dm,spname,split_vars);CHKERRQ(ierr);
      }
      {
	std::set<subdomain_id_type> blocks;
	ierr = DMMooseGetMeshBlocks_Private(dm,blocks);CHKERRQ(ierr);
	PetscInt maxblocks = blocks.size();
	char** splitblocks;
	ierr = PetscMalloc(maxblocks*sizeof(char*),&splitblocks);CHKERRQ(ierr);
	std::set<std::string> split_blocks;
	PetscInt nblocks = maxblocks;
	std::string opt = "-dm_moose_fieldsplit_"+*sit+"_blocks";
	std::string help = "Blocks in fieldsplit "+*sit;
	ierr = PetscOptionsStringArray(opt.c_str(),help.c_str(),"DMMooseSetSplitBlocks",splitblocks,&nblocks,PETSC_NULL);CHKERRQ(ierr);
	for (PetscInt i = 0; i < nblocks; ++i) {
	  split_blocks.insert(std::string(splitblocks[i]));
	}
	ierr = PetscFree(splitblocks);CHKERRQ(ierr);
	if (split_blocks.size()) {
	  ierr = DMMooseSetSplitBlocks(dm,spname,split_blocks);CHKERRQ(ierr);
	}
      }
      {
	PetscInt maxsides = dmm->nl->sys().get_mesh().boundary_info->get_boundary_ids().size();
	char** splitsides;
	ierr = PetscMalloc(maxsides*sizeof(char*),&splitsides);CHKERRQ(ierr);
	std::set<std::string> split_sides;
	PetscInt nsides = maxsides;
	std::string opt = "-dm_moose_fieldsplit_"+*sit+"_sides";
	std::string help = "Sides in fieldsplitsplit "+*sit;
	ierr = PetscOptionsStringArray(opt.c_str(),help.c_str(),"DMMooseSetSplitSides",splitsides,&nsides,PETSC_NULL);CHKERRQ(ierr);
	for (PetscInt i = 0; i < nsides; ++i) {
	  split_sides.insert(std::string(splitsides[i]));
	}
	ierr = PetscFree(splitsides);CHKERRQ(ierr);
	ierr = DMMooseSetSplitSides(dm,spname,split_sides);CHKERRQ(ierr);
      }
    }
  }
  PetscInt nsubdomains = 0;
  /* Insert the usage of -dm_moose_subdomain_names and -dm_moose_subdomain_<subdomainname> into this help message, since the following if-clause might never fire, if -help is requested. */
  const char* ddhelp = "Number of named subdomains defined by the DM.\n\
                \tNames of subdomains are defined by -dm_moose_subdomain_names <subdomainname1> <subdomainname2> ...\n\
                \tEach subdomain defined by the mesh blocks that belong to it as follows: -dm_moose_subdomain_<subdomainname> <block1> <block2> ...\n\
                \tIf no -dm_moose_subdomain_names is given, <subdomainname> is 0, 1, ...";
  ierr = PetscOptionsInt("-dm_moose_nsubdomains", ddhelp, "DMMooseSetDomainDecomposition", nsubdomains, &nsubdomains, NULL);CHKERRQ(ierr);
  if (nsubdomains) {
    PetscInt nnsubdomains = nsubdomains;
    std::set<std::string> subdomain_names;
    char** subdomainnames;
    ierr = PetscMalloc(nsubdomains*sizeof(char*),&subdomainnames);CHKERRQ(ierr);
    ierr = PetscOptionsStringArray("-dm_moose_subdomains_names", "Names of subdomains defined by the DM", "DMMooseSetDomainDecomposition", subdomainnames, &nnsubdomains, NULL);CHKERRQ(ierr);
    if (!nnsubdomains) {
      for (PetscInt i = 0; i < nsubdomains; ++i) {
	std::ostringstream s;
	s << i;
	subdomain_names.insert(s.str());
      }
    } else if (nsubdomains != nnsubdomains) {
      SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_SIZ, "Expected %D subdomain names, got %D instead", nsubdomains, nnsubdomains);
    } else {
      for (PetscInt i = 0; i < nsubdomains; ++i) {
	subdomain_names.insert(std::string(subdomainnames[i]));
      }
    }
    ierr = PetscFree(subdomainnames);CHKERRQ(ierr);
    PetscInt maxblocks = dmm->nl->sys().get_mesh().n_subdomains();
    char** subdomainblocks;
    std::map<std::string, std::set<std::string> > domain_decomposition;
    ierr = PetscMalloc(maxblocks*sizeof(char*),&subdomainblocks);CHKERRQ(ierr);
    for (std::set<std::string>::const_iterator sit = subdomain_names.begin(); sit != subdomain_names.end(); ++sit) {
      domain_decomposition[*sit] = std::set<std::string>();
      PetscInt nblocks = maxblocks;
      std::string opt = "-dm_moose_subdomain_"+*sit;
      std::string help = "Blocks in subdomain "+*sit;
      ierr = PetscOptionsStringArray(opt.c_str(),help.c_str(),"DMMooseSetDomainDecomposition",subdomainblocks,&nblocks,NULL);CHKERRQ(ierr);
      for (PetscInt i = 0; i < nblocks; ++i) {
	domain_decomposition[*sit].insert(std::string(subdomainblocks[i]));
      }
    }
    ierr = PetscFree(subdomainblocks);CHKERRQ(ierr);
    ierr = DMMooseSetDomainDecomposition(dm,domain_decomposition);CHKERRQ(ierr);
  }
  ierr = PetscOptionsEnd();CHKERRQ(ierr);
  ierr = DMSetUp_Moose_Private(dm);CHKERRQ(ierr); /* Because, strangely enough, DMView() is called in DMSetFromOptions(). */
  PetscFunctionReturn(0);

}



#undef __FUNCT__
#define __FUNCT__ "DMDestroy_Moose"
static PetscErrorCode  DMDestroy_Moose(DM dm)
{
  DM_Moose *dmm = (DM_Moose*)(dm->data);
  PetscErrorCode ierr;

  PetscFunctionBegin;
  /* CONTINUE: guard for any null pointers here? */
  if (dmm->vars) delete dmm->vars;
  delete dmm->varids;
  delete dmm->varnames;
  if (dmm->blocks) delete dmm->blocks;
  delete dmm->blockids;
  delete dmm->blocknames;
  if (dmm->sides) delete dmm->sides;
  delete dmm->sideids;
  delete dmm->sidenames;
  if(dmm->splits) delete dmm->splits;
  if(dmm->field_decomposition)  delete dmm->field_decomposition;
  if(dmm->domain_decomposition) delete dmm->domain_decomposition;
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

  dmm->varids     = new(std::map<std::string, unsigned int>);
  dmm->blockids   = new(std::map<std::string, unsigned int>);
  dmm->varnames   = new(std::map<unsigned int, std::string>);
  dmm->blocknames = new(std::map<unsigned int, std::string>);
  dmm->sideids     = new(std::map<std::string, BoundaryID>);
  dmm->sidenames   = new(std::map<BoundaryID,std::string>);

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


#endif // #if !PETSC_VERSION_LESS_THAN(3,3,0)
