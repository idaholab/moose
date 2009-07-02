//Moose Includes
#include "Moose.h"
#include "UpdateAuxVars.h"
#include "KernelFactory.h"
#include "MaterialFactory.h"
#include "BoundaryCondition.h"
#include "BCFactory.h"
#include "ParallelUniqueId.h"

//libMesh includes
#include "numeric_vector.h"
#include "dense_vector.h"
#include "petsc_matrix.h"
#include "dof_map.h"
#include "mesh.h"
#include "boundary_info.h"
#include "elem_range.h"

#include <vector>

class ComputeInternalResiduals
{
public:
  ComputeInternalResiduals(const NumericVector<Number>& in_soln, NumericVector<Number>& in_residual)
    :soln(in_soln),
     residual(in_residual)
  {}

  void operator() (const ConstElemRange & range) const
  {
    ParallelUniqueId puid;

    unsigned int tid = puid.id;

    DenseVector<Number> Re;

    ConstElemRange::const_iterator el = range.begin();

    KernelIterator kernel_begin = KernelFactory::instance()->activeKernelsBegin(tid);
    KernelIterator kernel_end = KernelFactory::instance()->activeKernelsEnd(tid);
    KernelIterator kernel_it = kernel_begin;

    KernelIterator block_kernel_begin;
    KernelIterator block_kernel_end;
    KernelIterator block_kernel_it;

    unsigned int subdomain = 999999999;

    for (el = range.begin() ; el != range.end(); ++el)
    {
      const Elem* elem = *el;

      Re.zero();

      Kernel::reinit(tid, soln, elem, &Re);

      unsigned int cur_subdomain = elem->subdomain_id();

      if(cur_subdomain != subdomain)
      {
        subdomain = cur_subdomain;

        Material * material = MaterialFactory::instance()->getMaterial(tid, subdomain);
        material->subdomainSetup();

        block_kernel_begin = KernelFactory::instance()->blockKernelsBegin(tid, subdomain);
        block_kernel_end = KernelFactory::instance()->blockKernelsEnd(tid, subdomain);

        //Global Kernels
        for(kernel_it=kernel_begin;kernel_it!=kernel_end;kernel_it++)
          (*kernel_it)->subdomainSetup();

        //Kernels on this block
        for(block_kernel_it=block_kernel_begin;block_kernel_it!=block_kernel_end;block_kernel_it++)
          (*block_kernel_it)->subdomainSetup();        
      } 

      //Global Kernels
      for(kernel_it=kernel_begin;kernel_it!=kernel_end;++kernel_it)
        (*kernel_it)->computeResidual();

      //Kernels on this block
      for(block_kernel_it=block_kernel_begin;block_kernel_it!=block_kernel_end;++block_kernel_it)
        (*block_kernel_it)->computeResidual();

      for (unsigned int side=0; side<elem->n_sides(); side++)
      {
        if (elem->neighbor(side) == NULL)
        {
          unsigned int boundary_id = Moose::mesh->boundary_info->boundary_id (elem, side);

          BCIterator bc_it = BCFactory::instance()->activeBCsBegin(tid,boundary_id);
          BCIterator bc_end = BCFactory::instance()->activeBCsEnd(tid,boundary_id);

          if(bc_it != bc_end)
          {
            BoundaryCondition::reinit(tid, soln, side, boundary_id);
          
            for(; bc_it!=bc_end; ++bc_it)
              (*bc_it)->computeResidual();
          }
        }
      }
      
      Kernel::_dof_map->constrain_element_vector (Re, Kernel::_dof_indices[tid], false);

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx); 
        residual.add_vector(Re, Kernel::_dof_indices[tid]);
      }
    }

  }

protected:
  const NumericVector<Number>& soln;
  NumericVector<Number>& residual;
};

      
      


namespace Moose
{
  void compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual)
  {
    Moose::perf_log.push("compute_residual()","Solve");

    residual.zero();

    update_aux_vars(soln);  

    static ConstElemRange elem_range(Moose::mesh->active_local_elements_begin(),
                                     Moose::mesh->active_local_elements_end(),1);
    
    Threads::parallel_for(elem_range,
                          ComputeInternalResiduals(soln, residual));

    residual.close();

    //Dirichlet BCs
    std::vector<unsigned int> nodes;
    std::vector<short int> ids;

    mesh->boundary_info->build_node_list(nodes, ids);
  
    const unsigned int n_nodes = nodes.size();

    for(unsigned int i=0; i<n_nodes; i++)
    {
      unsigned int boundary_id = ids[i];
    
      BCIterator bc_it = BCFactory::instance()->activeNodalBCsBegin(0,boundary_id);
      BCIterator bc_end = BCFactory::instance()->activeNodalBCsEnd(0,boundary_id);

      if(bc_it != bc_end)
      {
        Node & node = mesh->node(nodes[i]);

        if(node.processor_id() == libMesh::processor_id())
        {
          BoundaryCondition::reinit(0, soln, node, boundary_id, residual);

          for(; bc_it != bc_end; ++bc_it)
            (*bc_it)->computeAndStoreResidual();
        }
      }
    }


//    Parallel::barrier();
//    residual.print();
  //  u_system->rhs->print();

    residual.close();

    
    Moose::perf_log.pop("compute_residual()","Solve");
  }
}
