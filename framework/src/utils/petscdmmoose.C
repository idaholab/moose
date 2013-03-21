#include "libmesh/petsc_macro.h"
// This only works with petsc-3.3 and above.

#if !PETSC_VERSION_LESS_THAN(3,3,0)

#include "petscdmmoose.h"

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

using namespace libMesh;

struct DM_Moose
{
  NonlinearSystem*                      nl;
  std::set<std::string>                *vars;
  std::map<std::string, unsigned int>  *varids;
  std::map<unsigned int, std::string>  *varnames;
  std::set<std::string>                *blocks;
  std::map<std::string, unsigned int>  *blockids;
  std::map<unsigned int, std::string>  *blocknames;
  std::map<std::string,std::set<std::string> > *splits;
  std::map<std::string,std::set<unsigned int> > *field_decomposition;
  std::map<std::string,std::set<std::string> > *subdomains;
  std::map<std::string,std::set<unsigned int> > *domain_decomposition;
  IS                                    embedding;
};

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
#define __FUNCT__ "DMMooseSetFieldDecomposition"
PetscErrorCode DMMooseSetFieldDecomposition(DM dm, const std::map<std::string, std::set<std::string> >& splits)
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
  dmm->splits = new std::map<std::string, std::set<std::string> >(splits);
  PetscFunctionReturn(0);
}

#undef  __FUNCT__
#define __FUNCT__ "DMMooseGetFieldDecomposition"
PetscErrorCode DMMooseGetFieldDecomposition(DM dm, std::map<std::string, std::set<std::string> >& splits)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm,DM_CLASSID,1);
  PetscBool ismoose;
  ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE,&ismoose);CHKERRQ(ierr);
  if(!ismoose) SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_WRONG, "Got DM oftype %s, not of type %s", ((PetscObject)dm)->type_name, DMMOOSE);
  DM_Moose *dmm = (DM_Moose *)(dm->data);

  splits.clear();
  if (!dmm->field_decomposition || !dmm->field_decomposition->size()) PetscFunctionReturn(0);
  for (std::map<std::string, std::set<unsigned int> >::const_iterator sit = dmm->field_decomposition->begin(); sit != dmm->field_decomposition->end(); ++sit) {
    std::string sname = sit->first;
    splits[sname] = std::set<std::string>();
    for (std::set<unsigned int>::const_iterator it = (*dmm->field_decomposition)[sname].begin(); it != (*dmm->field_decomposition)[sname].end(); ++it) {
      splits[sname].insert((*dmm->varnames)[*it]);
    }
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
  std::string suffix = subname+"_";
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
  for (std::map<std::string, std::set<unsigned int> >::const_iterator dit = dmm->field_decomposition->begin(); dit != dmm->field_decomposition->end(); ++dit,++d) {
    std::string                          dname = dit->first;
    std::set<std::string>                dvars;
    std::set<std::string>                dblocks;
    std::set<unsigned int>               dindices;
    for(std::set<unsigned int>::const_iterator dvit = dit->second.begin(); dvit != dit->second.end(); ++dvit){
      unsigned int v = *dvit;
      std::string vname = (*dmm->varnames)[v];
      dvars.insert(vname);
      if(!islist) continue;
      /* Iterate only over this DM's blocks. */
      for(std::map<std::string, unsigned int>::const_iterator bit = dmm->blockids->begin(); bit != dmm->blockids->end(); ++bit) {
	unsigned int b = bit->second;
	dblocks.insert(bit->first);
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
      DM ddm;
      ierr = DMCreateMoose(((PetscObject)dm)->comm, *dmm->nl, &ddm);CHKERRQ(ierr);
      ierr = DMMooseSetVariables(ddm,dvars);CHKERRQ(ierr);
      ierr = DMMooseSetBlocks(ddm,dblocks);CHKERRQ(ierr);
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
  PetscVector<Number> X_global(x), R(r);

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
  PetscVector<Number>  X_global(x);

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
  PetscVector<Number> XL(xl);
  PetscVector<Number> XU(xu);

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
    ierr = PetscViewerASCIIPrintf(viewer, "blocks:", name, prefix); CHKERRQ(ierr);
    std::map<std::string,unsigned int>::iterator bit = dmm->blockids->begin();
    std::map<std::string,unsigned int>::const_iterator bend = dmm->blockids->end();
    for(; bit != bend; ++bit) {
      ierr = PetscViewerASCIIPrintf(viewer, "(%s,%D) ", bit->first.c_str(), bit->second); CHKERRQ(ierr);
    }
    ierr = PetscViewerASCIIPrintf(viewer, "\n"); CHKERRQ(ierr);
    ierr = PetscViewerASCIIPrintf(viewer, "variables:", name, prefix); CHKERRQ(ierr);
    std::map<std::string,unsigned int>::iterator vit = dmm->varids->begin();
    std::map<std::string,unsigned int>::const_iterator vend = dmm->varids->end();
    for(; vit != vend; ++vit) {
      ierr = PetscViewerASCIIPrintf(viewer, "(%s,%D) ", vit->first.c_str(), vit->second); CHKERRQ(ierr);
    }
    ierr = PetscViewerASCIIPrintf(viewer, "\n"); CHKERRQ(ierr);
    for (unsigned int i = 0; i < 2; ++i) {
      std::map<std::string, std::set<unsigned int> >  *decomposition = NULL;
      std::map<unsigned int, std::string>             *names = NULL;
      if (!i && dmm->field_decomposition) {
	ierr = PetscViewerASCIIPrintf(viewer, "Field decomposition by variable: ");CHKERRQ(ierr);
	decomposition = dmm->field_decomposition;
	names = dmm->varnames;
      } else if(dmm->domain_decomposition) {
	ierr = PetscViewerASCIIPrintf(viewer, "Domain decomposition by block: ");CHKERRQ(ierr);
	decomposition = dmm->domain_decomposition;
	names = dmm->blocknames;
      }
      /* FIX: decompositions might have different sizes and components on different ranks. */
      for (std::map<std::string, std::set<unsigned int> >::const_iterator dit = decomposition->begin(); dit != decomposition->end(); ++dit) {
	ierr = PetscViewerASCIIPrintf(viewer, "%s: ", dit->first.c_str()); CHKERRQ(ierr);
	for(std::set<unsigned int>::const_iterator it = dit->second.begin(); it != dit->second.end(); ++it) {
	  if(it != dit->second.begin()) {
	    ierr = PetscViewerASCIIPrintf(viewer, ",");CHKERRQ(ierr);
	  }
	  ierr = PetscViewerASCIIPrintf(viewer,"(%D,%s)",*it,(*names)[*it].c_str()); CHKERRQ(ierr);
	}
	ierr = PetscViewerASCIIPrintf(viewer, ";");CHKERRQ(ierr);
      }
      ierr = PetscViewerASCIIPrintf(viewer, "\n"); CHKERRQ(ierr);
    }
  }
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


  std::string name = dmm->nl->sys().name();
  name += "_v";
  for (std::map<unsigned int, std::string>::const_iterator vit = dmm->varnames->begin(); vit != dmm->varnames->end(); ++vit) {
    name += "_"+vit->second;
  }
  name += "_b";
  for (std::map<unsigned int, std::string>::const_iterator bit = dmm->blocknames->begin(); bit != dmm->blocknames->end(); ++bit) {
    name += "_"+bit->second;
  }

  /* Set up variables and blocks. */
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
  const MeshBase& mesh = dmm->nl->sys().get_mesh();
  dmm->blockids->clear();
  dmm->blocknames->clear();
  std::set<subdomain_id_type> blocks;
  /* The following effectively is a verbatim copy of MeshBase::n_subdomains(). */
  // This requires an inspection on every processor
  parallel_only();
  MeshBase::const_element_iterator       el  = mesh.active_elements_begin();
  const MeshBase::const_element_iterator end = mesh.active_elements_end();
  for (; el!=end; ++el)
    blocks.insert((*el)->subdomain_id());
  // Some subdomains may only live on other processors
  CommWorld.set_union(blocks);

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

  /*
     Set up decompositions.
   */
  if (dmm->splits) {
    dmm->field_decomposition = new std::map<std::string, std::set<unsigned int> >();
    unsigned int i = 0;
    for (std::map<std::string, std::set<std::string> >::const_iterator sit = dmm->splits->begin(); sit != dmm->splits->end(); ++sit, ++i) {
      unsigned int j = 0;
      for (std::set<std::string>::const_iterator it = sit->second.begin(); it != sit->second.end(); ++it, ++j) {
	std::string sname = sit->first;
	if (dmm->field_decomposition->count(sname)) {
	  SETERRQ1(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Duplicate split name: %s", sname.c_str());
	}
	(*dmm->field_decomposition)[sname] = std::set<unsigned int>();
	std::map<std::string, unsigned int>::const_iterator vit = dmm->varids->find(*it);
	if (vit == dmm->varids->end()) {
	  SETERRQ3(PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Unknown %D-th variable %s in split %D", i, it->c_str(), j);
	} else {
	  (*dmm->field_decomposition)[sname].insert(vit->second);
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
  /* Insert the usage of -dm_moose_split_names and -dm_moose_split_<splitname> into this help message, since the following if-clause might never fire, if -help is requested. */
  const char* fdhelp = "Number of named field splits defined by the DM.\n\
                \tNames of splits are defined by -dm_moose_split_names <splitname1> <splitname2> ...\n\
                \tEach split defined by the variables that belong to it as follows: -dm_moose_split_<splitname> <var1> <var2> ...\n\
                \tIf no -dm_moose_split_names is given, <splitname> is 0, 1, ...";
  ierr = PetscOptionsInt("-dm_moose_nsplits", fdhelp, "DMMooseSetFieldDecomposition", nsplits, &nsplits, NULL);CHKERRQ(ierr);
  if (nsplits) {
    PetscInt nnsplits = nsplits;
    std::set<std::string> split_names;
    char** splitnames;
    ierr = PetscMalloc(nsplits*sizeof(char*),&splitnames);CHKERRQ(ierr);
    ierr = PetscOptionsStringArray("-dm_moose_splits_names", "Names of splits defined by the DM", "DMMooseSetFieldDecomposition", splitnames, &nnsplits, NULL);CHKERRQ(ierr);
    if (!nnsplits) {
      for (PetscInt i = 0; i < nsplits; ++i) {
	std::ostringstream s;
	s << i;
	split_names.insert(s.str());
      }
    } else if (nsplits != nnsplits) {
      SETERRQ2(((PetscObject)dm)->comm, PETSC_ERR_ARG_SIZ, "Expected %D split names, got %D instead", nsplits, nnsplits);
    } else {
      for (PetscInt i = 0; i < nsplits; ++i) {
	split_names.insert(std::string(splitnames[i]));
      }
    }
    ierr = PetscFree(splitnames);CHKERRQ(ierr);
    PetscInt maxvars = dmm->nl->sys().get_dof_map().n_variables();
    char** splitvars;
    std::map<std::string, std::set<std::string> > field_decomposition;
    ierr = PetscMalloc(maxvars*sizeof(char*),&splitvars);CHKERRQ(ierr);
    for (std::set<std::string>::const_iterator sit = split_names.begin(); sit != split_names.end(); ++sit) {
      field_decomposition[*sit] = std::set<std::string>();
      PetscInt nvars = maxvars;
      std::string opt = "-dm_moose_split_"+*sit;
      std::string help = "Variables in split "+*sit;
      ierr = PetscOptionsStringArray(opt.c_str(),help.c_str(),"DMMooseSetFieldDecomposition",splitvars,&nvars,PETSC_NULL);CHKERRQ(ierr);
      for (PetscInt i = 0; i < nvars; ++i) {
	field_decomposition[*sit].insert(std::string(splitvars[i]));
      }
    }
    ierr = PetscFree(splitvars);CHKERRQ(ierr);
    ierr = DMMooseSetFieldDecomposition(dm,field_decomposition);CHKERRQ(ierr);
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
  PetscFunctionReturn(0);

}



#undef __FUNCT__
#define __FUNCT__ "DMDestroy_Moose"
static PetscErrorCode  DMDestroy_Moose(DM dm)
{
  DM_Moose *dmm = (DM_Moose*)(dm->data);
  PetscErrorCode ierr;

  PetscFunctionBegin;
  delete dmm->varids;
  delete dmm->varnames;
  delete dmm->blockids;
  delete dmm->blocknames;
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
