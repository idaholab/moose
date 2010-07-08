//Moose Includes
#include "AuxKernel.h"
#include "AuxFactory.h"
#include "Moose.h"
#include "MooseSystem.h"

//libMesh includes
#include "numeric_vector.h"
#include "mesh_base.h"
#include "mesh.h"
#include "boundary_info.h"

void MooseSystem::update_aux_vars(const NumericVector<Number>& soln)
{
  //If there aren't any auxiliary variables just return
  if(!_es->get_system<ExplicitSystem>("AuxiliarySystem").n_vars())
    return;

  Moose::perf_log.push("update_aux_vars()","Solve");

  //Nodal AuxKernels

  MeshBase::const_node_iterator nd     = _mesh->local_nodes_begin();
  MeshBase::const_node_iterator nd_end = _mesh->local_nodes_end();

  AuxKernelIterator aux_begin = _auxs.activeNodalAuxKernelsBegin(0);
  AuxKernelIterator aux_end = _auxs.activeNodalAuxKernelsEnd(0);
  AuxKernelIterator aux_it = aux_begin;

  if(aux_begin != aux_end)
  {
    for(;nd != nd_end; ++nd)
    {
      Node * node = *nd;

      reinitAuxKernels(0, soln, *node);

      for(aux_it = aux_begin; aux_it != aux_end; ++aux_it)
        (*aux_it)->computeAndStore(0);
    }
  }

  //Boundary AuxKernels
  std::vector<unsigned int> nodes;
  std::vector<short int> ids;

  _mesh->boundary_info->build_node_list(nodes, ids);

  const unsigned int n_nodes = nodes.size();

  for(unsigned int i=0; i<n_nodes; i++)
  {
    aux_begin = _auxs.activeAuxBCsBegin(0,ids[i]);
    aux_end = _auxs.activeAuxBCsEnd(0,ids[i]);

    if(aux_begin != aux_end)
    {
      Node & node = _mesh->node(nodes[i]);

      reinitAuxKernels(0, soln, node);

      for(aux_it=aux_begin; aux_it != aux_end; ++aux_it)
        (*aux_it)->computeAndStore(0);
    }
  }

  // FIXME: use _nl_system directly?
  _es->get_system<ExplicitSystem>("AuxiliarySystem").update();

  // Update the element aux vars
  aux_begin = _auxs.activeElementAuxKernelsBegin(0);
  aux_end = _auxs.activeElementAuxKernelsEnd(0);
  aux_it = aux_begin;

  MeshBase::const_element_iterator       el     = _mesh->active_local_elements_begin();
  const MeshBase::const_element_iterator end_el = _mesh->active_local_elements_end();

  unsigned int subdomain = 999999999;

  if(aux_begin != aux_end)
  {
    for ( ; el != end_el; ++el)
    {
      const Elem* elem = *el;

      reinitKernels(0, soln, elem, NULL);

      reinitAuxKernels(0, soln, *elem);

      unsigned int cur_subdomain = elem->subdomain_id();

      if(cur_subdomain != subdomain)
      {
        subdomain = cur_subdomain;
      
        _element_data._material[0] = _materials.getMaterials(0, subdomain);
        for(aux_it=aux_begin;aux_it!=aux_end;aux_it++)
          (*aux_it)->subdomainSetup();
      }

      for(aux_it=aux_begin;aux_it!=aux_end;aux_it++)
        (*aux_it)->computeAndStore(0);
    }
  }

  // FIXME: use _nl_system directly?
  _es->get_system<ExplicitSystem>("AuxiliarySystem").solution->close();
  _es->get_system<ExplicitSystem>("AuxiliarySystem").update();

  Moose::perf_log.pop("update_aux_vars()","Solve");
}
