//Moose Includes
#include "AuxKernel.h"
#include "AuxFactory.h"
#include "Moose.h"

//libMesh includes
#include "numeric_vector.h"
#include "mesh_base.h"
#include "mesh.h"
#include "boundary_info.h"

namespace Moose
{
  void update_aux_vars(const NumericVector<Number>& soln)
  {
    Moose::perf_log.push("update_aux_vars()","Solve");

    //Nodal AuxKernels

    MeshBase::const_node_iterator nd     = mesh->local_nodes_begin();
    MeshBase::const_node_iterator nd_end = mesh->local_nodes_end();

    std::vector<AuxKernel *>::iterator aux_begin = AuxFactory::instance()->activeNodalAuxKernelsBegin();
    std::vector<AuxKernel *>::iterator aux_end = AuxFactory::instance()->activeNodalAuxKernelsEnd();
    std::vector<AuxKernel *>::iterator aux_it = aux_begin;

    if(aux_begin != aux_end)
    {  
      for(;nd != nd_end; ++nd)
      {
        Node * node = *nd;

        AuxKernel::reinit(soln, *node);
      
        for(aux_it = aux_begin; aux_it != aux_end; ++aux_it)
          (*aux_it)->computeAndStore();
      }
    }

    //Boundary AuxKernels
    std::vector<unsigned int> nodes;
    std::vector<short int> ids;

    mesh->boundary_info->build_node_list(nodes, ids);
  
    const unsigned int n_nodes = nodes.size();

    for(unsigned int i=0; i<n_nodes; i++)
    {
      aux_begin = AuxFactory::instance()->activeAuxBCsBegin(ids[i]);
      aux_end = AuxFactory::instance()->activeAuxBCsEnd(ids[i]);

      if(aux_begin != aux_end)
      {
        Node & node = mesh->node(nodes[i]);
        AuxKernel::reinit(soln, node);

        for(aux_it=aux_begin; aux_it != aux_end; ++aux_it)
          (*aux_it)->computeAndStore();
      }
    }


    // Update the element aux vars
    aux_begin = AuxFactory::instance()->activeElementAuxKernelsBegin();
    aux_end = AuxFactory::instance()->activeElementAuxKernelsEnd();
    aux_it = aux_begin;
  
    MeshBase::const_element_iterator       el     = mesh->active_local_elements_begin();
    const MeshBase::const_element_iterator end_el = mesh->active_local_elements_end();

    unsigned int subdomain = 999999999;

    if(aux_begin != aux_end)
    {
      for ( ; el != end_el; ++el)
      {
        const Elem* elem = *el;
      
        Kernel::reinit(soln, elem, NULL);
      
        AuxKernel::reinit(soln, *elem);
      
        unsigned int cur_subdomain = elem->subdomain_id();
      
        if(cur_subdomain != subdomain)
        {
          subdomain = cur_subdomain;
        
          for(aux_it=aux_begin;aux_it!=aux_end;aux_it++)
            (*aux_it)->subdomainSetup();
        }
      
        for(aux_it=aux_begin;aux_it!=aux_end;aux_it++)
          (*aux_it)->computeAndStore();
      }
    }

    equation_system->get_system<ExplicitSystem>("AuxiliarySystem").solution->close();
    equation_system->get_system<ExplicitSystem>("AuxiliarySystem").update();

    Moose::perf_log.pop("update_aux_vars()","Solve");
  }
}

