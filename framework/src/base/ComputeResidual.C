//Moose Includes
#include "Moose.h"
#include "MaterialFactory.h"
#include "BoundaryCondition.h"
#include "ParallelUniqueId.h"
#include "MooseSystem.h"

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
  ComputeInternalResiduals(MooseSystem &sys, const NumericVector<Number>& in_soln, NumericVector<Number>& in_residual)
    :sys(sys),
     soln(in_soln),
     residual(in_residual)
  {}

  void operator() (const ConstElemRange & range) const
  {
    ParallelUniqueId puid;

    unsigned int tid = puid.id;

    DenseVector<Number> Re;

    ConstElemRange::const_iterator el = range.begin();

    sys._kernels.updateActiveKernels(tid);

    KernelIterator kernel_begin = sys._kernels.activeKernelsBegin(tid);
    KernelIterator kernel_end = sys._kernels.activeKernelsEnd(tid);
    KernelIterator kernel_it = kernel_begin;

    KernelIterator block_kernel_begin;
    KernelIterator block_kernel_end;
    KernelIterator block_kernel_it;

    StabilizerIterator stabilizer_begin = sys._stabilizers.activeStabilizersBegin(tid);
    StabilizerIterator stabilizer_end = sys._stabilizers.activeStabilizersEnd(tid);
    StabilizerIterator stabilizer_it = stabilizer_begin;

    unsigned int subdomain = 999999999;

    for (el = range.begin() ; el != range.end(); ++el)
    {
      const Elem* elem = *el;

      Re.zero();

      sys.reinitKernels(tid, soln, elem, &Re);

      unsigned int cur_subdomain = elem->subdomain_id();

      if(cur_subdomain != subdomain)
      {
        subdomain = cur_subdomain;

        Material * material = sys._materials.getMaterial(tid, subdomain);
        material->subdomainSetup();

        block_kernel_begin = sys._kernels.blockKernelsBegin(tid, subdomain);
        block_kernel_end = sys._kernels.blockKernelsEnd(tid, subdomain);

        //Global Kernels
        for(kernel_it=kernel_begin;kernel_it!=kernel_end;kernel_it++)
          (*kernel_it)->subdomainSetup();

        //Kernels on this block
        for(block_kernel_it=block_kernel_begin;block_kernel_it!=block_kernel_end;block_kernel_it++)
          (*block_kernel_it)->subdomainSetup();

        //Stabilizers
        for(stabilizer_it=stabilizer_begin;stabilizer_it!=stabilizer_end;stabilizer_it++)
          stabilizer_it->second->subdomainSetup();
      } 

      //Stabilizers
      for(stabilizer_it=stabilizer_begin;stabilizer_it!=stabilizer_end;stabilizer_it++)
        stabilizer_it->second->computeTestFunctions();

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
          unsigned int boundary_id = sys._mesh->boundary_info->boundary_id (elem, side);

          BCIterator bc_it = sys._bcs.activeBCsBegin(tid,boundary_id);
          BCIterator bc_end = sys._bcs.activeBCsEnd(tid,boundary_id);

          if(bc_it != bc_end)
          {
            sys.reinitBCs(tid, soln, side, boundary_id);
          
            for(; bc_it!=bc_end; ++bc_it)
              (*bc_it)->computeResidual();
          }
        }
      }
      
      sys._dof_map->constrain_element_vector (Re, sys._dof_indices[tid], false);

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx); 
        residual.add_vector(Re, sys._dof_indices[tid]);
      }
    }

  }

protected:
  MooseSystem &sys;
  const NumericVector<Number>& soln;
  NumericVector<Number>& residual;
};


namespace Moose {

void compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual)
{
  g_system->compute_residual(soln, residual);
}

}
      


void MooseSystem::compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual)
{
  Moose::perf_log.push("compute_residual()","Solve");

  residual.zero();

  update_aux_vars(soln);

  Threads::parallel_for(*getActiveLocalElementRange(),
                        ComputeInternalResiduals(*this, soln, residual));

  residual.close();

  //Dirichlet BCs
  std::vector<unsigned int> nodes;
  std::vector<short int> ids;

  _mesh->boundary_info->build_node_list(nodes, ids);

  const unsigned int n_nodes = nodes.size();

  for(unsigned int i=0; i<n_nodes; i++)
  {
    unsigned int boundary_id = ids[i];
  
    BCIterator bc_it = _bcs.activeNodalBCsBegin(0,boundary_id);
    BCIterator bc_end = _bcs.activeNodalBCsEnd(0,boundary_id);

    if(bc_it != bc_end)
    {
      Node & node = _mesh->node(nodes[i]);

      if(node.processor_id() == libMesh::processor_id())
      {
        reinitBCs(0, soln, node, boundary_id, residual);

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

