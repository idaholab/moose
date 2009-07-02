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
#include "petsc_matrix.h"
#include "sparse_matrix.h"
#include "dof_map.h"
#include "mesh.h"
#include "boundary_info.h"
#include "elem_range.h"

#include <vector>

class ComputeInternalJacobians
{
public:
  ComputeInternalJacobians(const NumericVector<Number>& in_soln, SparseMatrix<Number>&  in_jacobian)
    :soln(in_soln),
     jacobian(in_jacobian)
  {}
  
  void operator() (const ConstElemRange & range) const
  {
    ParallelUniqueId puid;

    unsigned int tid = puid.id;
    
    DenseMatrix<Number> Ke;

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

      Kernel::reinit(tid, soln, elem, NULL, &Ke);

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
      for(kernel_it=kernel_begin;kernel_it!=kernel_end;kernel_it++)
        (*kernel_it)->computeJacobian();

      //Kernels on this block
      for(block_kernel_it=block_kernel_begin;block_kernel_it!=block_kernel_end;block_kernel_it++)
        (*block_kernel_it)->computeJacobian();

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
              (*bc_it)->computeJacobian();
          }
        }
      }
    
      Kernel::_dof_map->constrain_element_matrix (Ke, Kernel::_dof_indices[tid], false);

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        jacobian.add_matrix(Ke, Kernel::_dof_indices[tid]);
      }
    }

  }

protected:
  const NumericVector<Number>& soln;
  SparseMatrix<Number>& jacobian;
};

namespace Moose
{
  
  void compute_jacobian (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian)
  {
    Moose::perf_log.push("compute_jacobian()","Solve");

#ifdef LIBMESH_HAVE_PETSC
    //Necessary for speed
    MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),MAT_KEEP_ZEROED_ROWS);
#endif
  
    update_aux_vars(soln);

    Threads::parallel_for(ConstElemRange(Moose::mesh->active_local_elements_begin(),
                                         Moose::mesh->active_local_elements_end(),1),
                          ComputeInternalJacobians(soln, jacobian));

    jacobian.close();

    //Dirichlet BCs
    std::vector<int> zero_rows;
    
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
            for(; bc_it != bc_end; ++bc_it)
              //The first zero is for the system
              //The second zero only works with Lagrange elements!
              zero_rows.push_back(node.dof_number(0, (*bc_it)->variable(), 0));
          }
      }
    }

    //This zeroes the rows corresponding to Dirichlet BCs and puts a 1.0 on the diagonal
    jacobian.zero_rows(zero_rows, 1.0);

    jacobian.close();
  
    Moose::perf_log.pop("compute_jacobian()","Solve");
  }

class ComputeInternalJacobianBlocks
{
public:
  ComputeInternalJacobianBlocks(const NumericVector<Number>& in_soln, SparseMatrix<Number>&  in_jacobian, System& in_precond_system, unsigned int & in_ivar, unsigned int & in_jvar)
    :soln(in_soln),
     jacobian(in_jacobian),
     precond_system(in_precond_system),
     ivar(in_ivar),
     jvar(in_jvar)
  {}
  
  void operator() (const ConstElemRange & range) const
  {
    ParallelUniqueId puid;

    unsigned int tid = puid.id;
    
    ConstElemRange::const_iterator el = range.begin();

    KernelIterator kernel_begin = KernelFactory::instance()->activeKernelsBegin(tid);
    KernelIterator kernel_end = KernelFactory::instance()->activeKernelsEnd(tid);
    KernelIterator kernel_it = kernel_begin;

    KernelIterator block_kernel_begin;
    KernelIterator block_kernel_end;
    KernelIterator block_kernel_it;

    unsigned int subdomain = 999999999;

    DofMap & dof_map = precond_system.get_dof_map();
    DenseMatrix<Number> Ke;
    std::vector<unsigned int> dof_indices;

    jacobian.zero();

    for (el = range.begin() ; el != range.end(); ++el)
    {
      const Elem* elem = *el;

      Kernel::reinit(tid, soln, elem, NULL, NULL);

      dof_map.dof_indices(elem, dof_indices);

      Ke.resize(dof_indices.size(),dof_indices.size());

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
      for(kernel_it=kernel_begin;kernel_it!=kernel_end;kernel_it++)
      {
        Kernel * kernel = *kernel_it;

        if(kernel->variable() == ivar)
          kernel->computeOffDiagJacobian(Ke,jvar);
      }

      //Kernels on this block
      for(block_kernel_it=block_kernel_begin;block_kernel_it!=block_kernel_end;block_kernel_it++)
      {
        Kernel * block_kernel = *block_kernel_it;

        if(block_kernel->variable() == ivar)
          block_kernel->computeOffDiagJacobian(Ke,jvar);
      }

      for (unsigned int side=0; side<elem->n_sides(); side++)
      {
        if (elem->neighbor(side) == NULL)
        {
          unsigned int boundary_id = mesh->boundary_info->boundary_id (elem, side);

          BCIterator bc_it = BCFactory::instance()->activeBCsBegin(tid,boundary_id);
          BCIterator bc_end = BCFactory::instance()->activeBCsEnd(tid,boundary_id);

          if(bc_it != bc_end)
          {
            BoundaryCondition::reinit(tid, soln, side, boundary_id);
          
            for(; bc_it!=bc_end; ++bc_it)
            {
              BoundaryCondition * bc = *bc_it;

              if(bc->variable() == ivar)
                bc->computeJacobianBlock(Ke,ivar,jvar);
            }
          }
        }
      }    

      dof_map.constrain_element_matrix (Ke, dof_indices, false);

      {  
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        jacobian.add_matrix(Ke, dof_indices);
      }   
    }
  }

protected:
  const NumericVector<Number>& soln;
  SparseMatrix<Number>& jacobian;
  System& precond_system;
  unsigned int & ivar;
  unsigned int & jvar;
};

  
  void compute_jacobian_block (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar)
  {
    Moose::perf_log.push("compute_jacobian_block()","Solve");

#ifdef LIBMESH_HAVE_PETSC
  //Necessary for speed
    MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),MAT_KEEP_ZEROED_ROWS);
#endif

    update_aux_vars(soln);
/*
    Threads::parallel_for(ConstElemRange(Moose::mesh->active_local_elements_begin(),
                                         Moose::mesh->active_local_elements_end(),1),
                          ComputeInternalJacobianBlocks(soln, jacobian, precond_system, ivar, jvar));
*/
    
  {
    unsigned int tid = 0;
    
    MeshBase::const_element_iterator el = Moose::mesh->active_local_elements_begin();
    const MeshBase::const_element_iterator end_el = Moose::mesh->active_local_elements_end();

    KernelIterator kernel_begin = KernelFactory::instance()->activeKernelsBegin(tid);
    KernelIterator kernel_end = KernelFactory::instance()->activeKernelsEnd(tid);
    KernelIterator kernel_it = kernel_begin;

    KernelIterator block_kernel_begin;
    KernelIterator block_kernel_end;
    KernelIterator block_kernel_it;

    unsigned int subdomain = 999999999;

    DofMap & dof_map = precond_system.get_dof_map();
    DenseMatrix<Number> Ke;
    std::vector<unsigned int> dof_indices;

    jacobian.zero();

    for (; el != end_el; ++el)
    {
      const Elem* elem = *el;

      Kernel::reinit(tid, soln, elem, NULL, NULL);

      dof_map.dof_indices(elem, dof_indices);

      Ke.resize(dof_indices.size(),dof_indices.size());

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
      for(kernel_it=kernel_begin;kernel_it!=kernel_end;kernel_it++)
      {
        Kernel * kernel = *kernel_it;

        if(kernel->variable() == ivar)
          kernel->computeOffDiagJacobian(Ke,jvar);
      }

      //Kernels on this block
      for(block_kernel_it=block_kernel_begin;block_kernel_it!=block_kernel_end;block_kernel_it++)
      {
        Kernel * block_kernel = *block_kernel_it;

        if(block_kernel->variable() == ivar)
          block_kernel->computeOffDiagJacobian(Ke,jvar);
      }

      for (unsigned int side=0; side<elem->n_sides(); side++)
      {
        if (elem->neighbor(side) == NULL)
        {
          unsigned int boundary_id = mesh->boundary_info->boundary_id (elem, side);

          BCIterator bc_it = BCFactory::instance()->activeBCsBegin(tid,boundary_id);
          BCIterator bc_end = BCFactory::instance()->activeBCsEnd(tid,boundary_id);

          if(bc_it != bc_end)
          {
            BoundaryCondition::reinit(tid, soln, side, boundary_id);
          
            for(; bc_it!=bc_end; ++bc_it)
            {
              BoundaryCondition * bc = *bc_it;

              if(bc->variable() == ivar)
                bc->computeJacobianBlock(Ke,ivar,jvar);
            }
          }
        }
      }    

      dof_map.constrain_element_matrix (Ke, dof_indices, false);

      jacobian.add_matrix(Ke, dof_indices);
    }
  }
    
    jacobian.close();

    //Dirichlet BCs
    std::vector<int> zero_rows;
    
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
          for(; bc_it != bc_end; ++bc_it)
            //The first zero is for the variable number... there is only one variable in each mini-system
            //The second zero only works with Lagrange elements!
            if((*bc_it)->variable() == ivar)
              zero_rows.push_back(node.dof_number(precond_system.number(), 0, 0));
        }
      }
    }

    jacobian.close();

    //This zeroes the rows corresponding to Dirichlet BCs and puts a 1.0 on the diagonal
    jacobian.zero_rows(zero_rows, 1.0);
  
    jacobian.close();
  
    Moose::perf_log.pop("compute_jacobian_block()","Solve");
  }

}
