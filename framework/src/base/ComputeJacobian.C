//Moose Includes
#include "Moose.h"
#include "KernelFactory.h"
#include "MaterialFactory.h"
#include "StabilizerFactory.h"
#include "BoundaryCondition.h"
#include "BCFactory.h"
#include "ParallelUniqueId.h"
#include "MooseSystem.h"
#include "ElementData.h"

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
  ComputeInternalJacobians(MooseSystem &sys, const NumericVector<Number>& in_soln, SparseMatrix<Number>&  in_jacobian)
    :_moose_system(sys),
     _soln(in_soln),
     _jacobian(in_jacobian)
  {}
  
  void operator() (const ConstElemRange & range) const
  {
    ParallelUniqueId puid;

    unsigned int tid = puid.id;
    
    DenseMatrix<Number> Ke;

    ConstElemRange::const_iterator el = range.begin();

    _moose_system._kernels.updateActiveKernels(tid);

    KernelIterator kernel_begin = _moose_system._kernels.activeKernelsBegin(tid);
    KernelIterator kernel_end = _moose_system._kernels.activeKernelsEnd(tid);
    KernelIterator kernel_it = kernel_begin;

    KernelIterator block_kernel_begin;
    KernelIterator block_kernel_end;
    KernelIterator block_kernel_it;

    StabilizerIterator stabilizer_begin = _moose_system._stabilizers.activeStabilizersBegin(tid);
    StabilizerIterator stabilizer_end = _moose_system._stabilizers.activeStabilizersEnd(tid);
    StabilizerIterator stabilizer_it = stabilizer_begin;

    unsigned int subdomain = 999999999;

    for (el = range.begin() ; el != range.end(); ++el)
    {
      const Elem* elem = *el;

      _moose_system.reinitKernels(tid, _soln, elem, NULL, &Ke);

      unsigned int cur_subdomain = elem->subdomain_id();

      if(cur_subdomain != subdomain)
      {
        subdomain = cur_subdomain;

        Material * material = _moose_system._materials.getMaterial(tid, subdomain);
        material->subdomainSetup();

        block_kernel_begin = _moose_system._kernels.blockKernelsBegin(tid, subdomain);
        block_kernel_end = _moose_system._kernels.blockKernelsEnd(tid, subdomain);
      
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
      for(kernel_it=kernel_begin;kernel_it!=kernel_end;kernel_it++)
        (*kernel_it)->computeJacobian();

      //Kernels on this block
      for(block_kernel_it=block_kernel_begin;block_kernel_it!=block_kernel_end;block_kernel_it++)
        (*block_kernel_it)->computeJacobian();

      for (unsigned int side=0; side<elem->n_sides(); side++)
      {
        if (elem->neighbor(side) == NULL)
        {
          unsigned int boundary_id = _moose_system._mesh->boundary_info->boundary_id (elem, side);

          BCIterator bc_it = _moose_system._bcs.activeBCsBegin(tid,boundary_id);
          BCIterator bc_end = _moose_system._bcs.activeBCsEnd(tid,boundary_id);

          if(bc_it != bc_end)
          {
            _moose_system.reinitBCs(tid, _soln, side, boundary_id);
          
            for(; bc_it!=bc_end; ++bc_it)
              (*bc_it)->computeJacobian();
          }
        }
      }    

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        for(unsigned int i=0; i< _moose_system._element_data->_var_dof_indices[tid].size(); i++)
        {
          _moose_system._element_data->_dof_map->constrain_element_matrix (*_moose_system._element_data->_var_Kes[tid][i], _moose_system._element_data->_var_dof_indices[tid][i], false);
          _jacobian.add_matrix(*_moose_system._element_data->_var_Kes[tid][i], _moose_system._element_data->_var_dof_indices[tid][i]);
        }
      }
    }

  }

protected:
  MooseSystem& _moose_system;
  const NumericVector<Number>& _soln;
  SparseMatrix<Number>& _jacobian;
};

namespace Moose {

void compute_jacobian (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian)
{
  g_system->compute_jacobian(soln, jacobian);
}

}

void MooseSystem::compute_jacobian (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian)
{
  Moose::perf_log.push("compute_jacobian()","Solve");

#ifdef LIBMESH_HAVE_PETSC
  //Necessary for speed
#if PETSC_VERSION_LESS_THAN(3,0,0)
  MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),MAT_KEEP_ZEROED_ROWS);
#else
  // In Petsc 3.0.0, MatSetOption has three args...the third arg
  // determines whether the option is set (true) or unset (false)
  MatSetOption(static_cast<PetscMatrix<Number> &>(_jacobian).mat(),
   MAT_KEEP_ZEROED_ROWS,
   PETSC_TRUE);
#endif
    
#endif
  
  update_aux_vars(soln);

  Threads::parallel_for(*getActiveLocalElementRange(),
                        ComputeInternalJacobians(*this, soln, jacobian));

  jacobian.close();

  //Dirichlet BCs
  std::vector<int> zero_rows;

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
  ComputeInternalJacobianBlocks(MooseSystem &sys, const NumericVector<Number>& in_soln, SparseMatrix<Number>&  in_jacobian, System& in_precond_system, unsigned int & in_ivar, unsigned int & in_jvar)
    :_moose_system(sys),
     _soln(in_soln),
     _jacobian(in_jacobian),
     _precond_system(in_precond_system),
     _ivar(in_ivar),
     _jvar(in_jvar)
  {}
  
  void operator() (const ConstElemRange & range) const
  {
    ParallelUniqueId puid;

    unsigned int tid = puid.id;
    
    ConstElemRange::const_iterator el = range.begin();

    _moose_system._kernels.updateActiveKernels(tid);

    KernelIterator kernel_begin = _moose_system._kernels.activeKernelsBegin(tid);
    KernelIterator kernel_end = _moose_system._kernels.activeKernelsEnd(tid);
    KernelIterator kernel_it = kernel_begin;

    KernelIterator block_kernel_begin;
    KernelIterator block_kernel_end;
    KernelIterator block_kernel_it;

    StabilizerIterator stabilizer_begin = _moose_system._stabilizers.activeStabilizersBegin(tid);
    StabilizerIterator stabilizer_end = _moose_system._stabilizers.activeStabilizersEnd(tid);
    StabilizerIterator stabilizer_it = stabilizer_begin;

    unsigned int subdomain = 999999999;

    DofMap & dof_map = _precond_system.get_dof_map();
    DenseMatrix<Number> Ke;
    std::vector<unsigned int> dof_indices;

    _jacobian.zero();

    for (el = range.begin() ; el != range.end(); ++el)
    {
      const Elem* elem = *el;

      _moose_system.reinitKernels(tid, _soln, elem, NULL, NULL);

      dof_map.dof_indices(elem, dof_indices);

      Ke.resize(dof_indices.size(),dof_indices.size());

      unsigned int cur_subdomain = elem->subdomain_id();

      if(cur_subdomain != subdomain)
      {
        subdomain = cur_subdomain;

        Material * material = _moose_system._materials.getMaterial(tid, subdomain);
        material->subdomainSetup();

        block_kernel_begin = _moose_system._kernels.blockKernelsBegin(tid, subdomain);
        block_kernel_end = _moose_system._kernels.blockKernelsEnd(tid, subdomain);
      
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
      for(kernel_it=kernel_begin;kernel_it!=kernel_end;kernel_it++)
      {
        Kernel * kernel = *kernel_it;

        if(kernel->variable() == _ivar)
          kernel->computeOffDiagJacobian(Ke,_jvar);
      }

      //Kernels on this block
      for(block_kernel_it=block_kernel_begin;block_kernel_it!=block_kernel_end;block_kernel_it++)
      {
        Kernel * block_kernel = *block_kernel_it;

        if(block_kernel->variable() == _ivar)
          block_kernel->computeOffDiagJacobian(Ke,_jvar);
      }

      for (unsigned int side=0; side<elem->n_sides(); side++)
      {
        if (elem->neighbor(side) == NULL)
        {
          unsigned int boundary_id = _moose_system._mesh->boundary_info->boundary_id (elem, side);

          BCIterator bc_it = _moose_system._bcs.activeBCsBegin(tid,boundary_id);
          BCIterator bc_end = _moose_system._bcs.activeBCsEnd(tid,boundary_id);

          if(bc_it != bc_end)
          {
            _moose_system.reinitBCs(tid, _soln, side, boundary_id);
          
            for(; bc_it!=bc_end; ++bc_it)
            {
              BoundaryCondition * bc = *bc_it;

              if(bc->variable() == _ivar)
                bc->computeJacobianBlock(Ke,_ivar,_jvar);
            }
          }
        }
      }    

      dof_map.constrain_element_matrix (Ke, dof_indices, false);

      {  
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _jacobian.add_matrix(Ke, dof_indices);
      }   
    }
  }

protected:
  MooseSystem& _moose_system;
  const NumericVector<Number>& _soln;
  SparseMatrix<Number>& _jacobian;
  System& _precond_system;
  unsigned int & _ivar;
  unsigned int & _jvar;
};

namespace Moose {

void compute_jacobian_block (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar)
{
  g_system->compute_jacobian_block (soln, jacobian, precond_system, ivar, jvar);
}

}

void MooseSystem::compute_jacobian_block (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar)
{
  Moose::perf_log.push("compute_jacobian_block()","Solve");

#ifdef LIBMESH_HAVE_PETSC
  //Necessary for speed
#if PETSC_VERSION_LESS_THAN(3,0,0)
  MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),MAT_KEEP_ZEROED_ROWS);
#else
  // In Petsc 3.0.0, MatSetOption has three args...the third arg
  // determines whether the option is set (true) or unset (false)
  MatSetOption(static_cast<PetscMatrix<Number> &>(_jacobian).mat(),
   MAT_KEEP_ZEROED_ROWS,
   PETSC_TRUE);
#endif
#endif

  update_aux_vars(soln);
/*
    Threads::parallel_for(ConstElemRange(Moose::mesh->active_local_elements_begin(),
                                         Moose::mesh->active_local_elements_end(),1),
                          ComputeInternalJacobianBlocks(soln, jacobian, precond_system, ivar, jvar));
*/
    
  {
    unsigned int tid = 0;
    
    MeshBase::const_element_iterator el = _mesh->active_local_elements_begin();
    const MeshBase::const_element_iterator end_el = _mesh->active_local_elements_end();

    _kernels.updateActiveKernels(tid);

    KernelIterator kernel_begin = _kernels.activeKernelsBegin(tid);
    KernelIterator kernel_end = _kernels.activeKernelsEnd(tid);
    KernelIterator kernel_it = kernel_begin;

    KernelIterator block_kernel_begin;
    KernelIterator block_kernel_end;
    KernelIterator block_kernel_it;

    StabilizerIterator stabilizer_begin = _stabilizers.activeStabilizersBegin(tid);
    StabilizerIterator stabilizer_end = _stabilizers.activeStabilizersEnd(tid);
    StabilizerIterator stabilizer_it = stabilizer_begin;

    unsigned int subdomain = 999999999;

    DofMap & dof_map = precond_system.get_dof_map();
    DenseMatrix<Number> Ke;
    std::vector<unsigned int> dof_indices;

    jacobian.zero();

    for (; el != end_el; ++el)
    {
      const Elem* elem = *el;

      reinitKernels(tid, soln, elem, NULL, NULL);

      dof_map.dof_indices(elem, dof_indices);

      Ke.resize(dof_indices.size(),dof_indices.size());

      unsigned int cur_subdomain = elem->subdomain_id();

      if(cur_subdomain != subdomain)
      {
        subdomain = cur_subdomain;

        Material * material = _materials.getMaterial(tid, subdomain);
        material->subdomainSetup();

        block_kernel_begin = _kernels.blockKernelsBegin(tid, subdomain);
        block_kernel_end = _kernels.blockKernelsEnd(tid, subdomain);
      
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
          unsigned int boundary_id = _mesh->boundary_info->boundary_id (elem, side);

          BCIterator bc_it = _bcs.activeBCsBegin(tid,boundary_id);
          BCIterator bc_end = _bcs.activeBCsEnd(tid,boundary_id);

          if(bc_it != bc_end)
          {
            reinitBCs(tid, soln, side, boundary_id);
          
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
