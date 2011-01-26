/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

//Moose Includes
#include "Moose.h"
#include "KernelFactory.h"
#include "MaterialFactory.h"
#include "StabilizerFactory.h"
#include "BoundaryCondition.h"
#include "BCFactory.h"
#include "ParallelUniqueId.h"
#include "MooseSystem.h"
#include "DofData.h"
#include "ElementData.h"
#include "ComputeBase.h"

//libMesh includes
#include "numeric_vector.h"
#include "petsc_matrix.h"
#include "sparse_matrix.h"
#include "dof_map.h"
#include "mesh.h"
#include "boundary_info.h"
#include "elem_range.h"

#include <vector>

class ComputeInternalJacobians : public ComputeBase<ConstElemRange>
{
public:
  ComputeInternalJacobians(MooseSystem &sys, const NumericVector<Number>& in_soln, SparseMatrix<Number>&  in_jacobian) :
    ComputeBase<ConstElemRange>(sys),
     _soln(in_soln),
     _jacobian(in_jacobian)
  {}

  // Splitting Constructor
  ComputeInternalJacobians(ComputeInternalJacobians & x, Threads::split) :
    ComputeBase<ConstElemRange>(x._moose_system),
    _soln(x._soln),
    _jacobian(x._jacobian)
  {
  }

  virtual void pre()
  {
    _moose_system._dg_kernels[_tid].updateActiveDGKernels(_moose_system._t, _moose_system._dt);
  }

  virtual void preElement(const Elem * elem)
  {
    _moose_system.reinitKernels(_tid, _soln, elem, NULL, &_ke);
  }

  virtual void onElement(const Elem * elem)
  {
    unsigned int cur_subdomain = elem->subdomain_id();
    _moose_system._element_data[_tid]->reinitMaterials(_moose_system._materials[_tid].getMaterials(cur_subdomain));

    //Stabilizers
    StabilizerIterator stabilizer_begin = _moose_system._stabilizers[_tid].activeStabilizersBegin();
    StabilizerIterator stabilizer_end = _moose_system._stabilizers[_tid].activeStabilizersEnd();
    StabilizerIterator stabilizer_it = stabilizer_begin;

    for(stabilizer_it=stabilizer_begin;stabilizer_it!=stabilizer_end;stabilizer_it++)
      stabilizer_it->second->computeTestFunctions();

    //Global Kernels
    KernelIterator kernel_begin = _moose_system._kernels[_tid].activeKernelsBegin();
    KernelIterator kernel_end = _moose_system._kernels[_tid].activeKernelsEnd();
    KernelIterator kernel_it = kernel_begin;

    for(kernel_it=kernel_begin;kernel_it!=kernel_end;kernel_it++)
      (*kernel_it)->computeJacobian();
  }

  virtual void postElement(const Elem * /*elem*/)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for(unsigned int i=0; i< _moose_system._dof_data[_tid]._var_dof_indices.size(); i++)
    {
      if(_moose_system._dof_data[_tid]._var_dof_indices[i].size())
      {
        _moose_system._dof_map->constrain_element_matrix (*_moose_system._dof_data[_tid]._var_Kes[i], _moose_system._dof_data[_tid]._var_dof_indices[i], false);
        _jacobian.add_matrix(*_moose_system._dof_data[_tid]._var_Kes[i], _moose_system._dof_data[_tid]._var_dof_indices[i]);
      }
    }
  }

  virtual void onDomainChanged(short int subdomain)
  {
    _moose_system.subdomainSetup(_tid, subdomain);
    _moose_system._kernels[_tid].updateActiveKernels(_moose_system._t, _moose_system._dt, subdomain);
  }

  virtual void onBoundary(const Elem * elem, unsigned int side, short int bnd_id)
  {
    BCIterator bc_it = _moose_system._bcs[_tid].activeBCsBegin(bnd_id);
    BCIterator bc_end = _moose_system._bcs[_tid].activeBCsEnd(bnd_id);

    if(bc_it != bc_end)
    {
      _moose_system.reinitBCs(_tid, _soln, elem, side, bnd_id);

      for(; bc_it!=bc_end; ++bc_it)
        if((*bc_it)->shouldBeApplied())
          (*bc_it)->computeJacobian();
    }
  }

  virtual void onInternalSide(const Elem * elem, unsigned int side)
  {
    // Pointer to the neighbor we are currently working on.
    const Elem * neighbor = elem->neighbor(side);

    // Get the global id of the element and the neighbor
    const unsigned int elem_id = elem->id();
    const unsigned int neighbor_id = neighbor->id();

    // If the neighbor has the same h level and is active
    // perform integration only if our global id is bigger than our neighbor id.
    // We don't want to compute twice the same contributions.
    // If the neighbor has a different h level perform integration
    // only if the neighbor is at a lower level.
    if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) || (neighbor->level() < elem->level()))
    {
      DGKernelIterator dg_it = _moose_system._dg_kernels[_tid].activeDGKernelsBegin();
      DGKernelIterator dg_end = _moose_system._dg_kernels[_tid].activeDGKernelsEnd();

      if (dg_it!=dg_end)
      {
        _moose_system.reinitDGKernels(_tid, _soln, elem, side, neighbor, NULL, true);

        for(; dg_it!=dg_end; ++dg_it)
          (*dg_it)->computeJacobian();

        {
          Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
          for(unsigned int i=0; i< _moose_system._neighbor_dof_data[_tid]._var_dof_indices.size(); i++)
          {
            std::vector<unsigned int> & e_dof_indices = _moose_system._dof_data[_tid]._var_dof_indices[i];
            std::vector<unsigned int> & n_dof_indices = _moose_system._neighbor_dof_data[_tid]._var_dof_indices[i];

            _moose_system._dof_map->constrain_element_matrix(*_moose_system._dof_data[_tid]._var_Kns[i], e_dof_indices, n_dof_indices);
            _moose_system._dof_map->constrain_element_matrix(*_moose_system._neighbor_dof_data[_tid]._var_Kns[i], n_dof_indices, e_dof_indices);
            _moose_system._dof_map->constrain_element_matrix(*_moose_system._neighbor_dof_data[_tid]._var_Kes[i], n_dof_indices, n_dof_indices);

            _jacobian.add_matrix(*_moose_system._dof_data[_tid]._var_Kns[i], e_dof_indices, n_dof_indices);
            _jacobian.add_matrix(*_moose_system._neighbor_dof_data[_tid]._var_Kns[i], n_dof_indices, e_dof_indices);
            _jacobian.add_matrix(*_moose_system._neighbor_dof_data[_tid]._var_Kes[i], n_dof_indices, n_dof_indices);
          }
        }
      }
    }
  }

  void join(const ComputeInternalJacobians & /*y*/)
  {
  }

protected:
  const NumericVector<Number>& _soln;
  SparseMatrix<Number>& _jacobian;

  DenseMatrix<Number> _ke;
};

namespace Moose {

void compute_jacobian (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, NonlinearImplicitSystem& sys)
{
  MooseSystem * moose_system = sys.get_equation_systems().parameters.get<MooseSystem *>("moose_system");

  moose_system->computeJacobian(soln, jacobian);
}

}

void MooseSystem::computeJacobian (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian)
{
  Moose::perf_log.push("compute_jacobian()","Solve");
  _current_jacobian = &jacobian;

  computePostprocessors(*(getNonlinearSystem()->current_local_solution), Moose::PPS_JACOBIAN);

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
  if(_serialize_solution)
    serializeSolution(soln);

  if(_has_displaced_mesh)
    updateDisplacedMesh(soln);
  
  updateAuxVars(soln);

  ComputeInternalJacobians cij(*this, soln, jacobian);
  Threads::parallel_reduce(*getActiveLocalElementRange(), cij);

  if(needJacobianCopy())
  {
    jacobian.close();  
    MatCopy(static_cast<PetscMatrix<Number> &>(jacobian).mat(), static_cast<PetscMatrix<Number> &>(*_jacobian_copy).mat(), DIFFERENT_NONZERO_PATTERN);
    _jacobian_copy->close();
  }

//  jacobian.print();
  
  //Distribute any point loads
  DUMMY_CONTACT_FLAG = true;
  computeDiracKernels(soln, NULL, &jacobian);
  DUMMY_CONTACT_FLAG = false;

//  jacobian.close();
//  jacobian.print();

  //Dirichlet BCs
  std::vector<int> zero_rows;

  std::vector<unsigned int> nodes;
  std::vector<short int> ids;

  _mesh->boundary_info->build_node_list(nodes, ids);

  const unsigned int n_nodes = nodes.size();

  for(unsigned int i=0; i<n_nodes; i++)
  {
    unsigned int boundary_id = ids[i];
  
    BCIterator bc_it = _bcs[0].activeNodalBCsBegin(boundary_id);
    BCIterator bc_end = _bcs[0].activeNodalBCsEnd(boundary_id);

    if(bc_it != bc_end)
    {
      Node & node = _mesh->node(nodes[i]);

      if(node.processor_id() == libMesh::processor_id())
      {
        for(; bc_it != bc_end; ++bc_it)
          //The first zero is for the system
          //The second zero only works with Lagrange elements!
          if((*bc_it)->shouldBeApplied())
            zero_rows.push_back(node.dof_number(0, (*bc_it)->variable(), 0));
      }
    }
  }

  //This zeroes the rows corresponding to Dirichlet BCs and puts a 1.0 on the diagonal
  jacobian.zero_rows(zero_rows, 1.0);

  jacobian.close();

//  jacobian.print();

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

    StabilizerIterator stabilizer_begin = _moose_system._stabilizers[tid].activeStabilizersBegin();
    StabilizerIterator stabilizer_end = _moose_system._stabilizers[tid].activeStabilizersEnd();
    StabilizerIterator stabilizer_it = stabilizer_begin;

    unsigned int subdomain = std::numeric_limits<unsigned int>::max();

    DofMap & dof_map = _precond_system.get_dof_map();
    DenseMatrix<Number> Ke;
    std::vector<unsigned int> dof_indices;

    _jacobian.zero();

    for (el = range.begin() ; el != range.end(); ++el)
    {
      const Elem* elem = *el;
      unsigned int cur_subdomain = elem->subdomain_id();

      _moose_system.reinitKernels(tid, _soln, elem, NULL, NULL);

      dof_map.dof_indices(elem, dof_indices);

      Ke.resize(dof_indices.size(),dof_indices.size());

      if(cur_subdomain != subdomain)
      {
        subdomain = cur_subdomain;
        _moose_system.subdomainSetup(tid, subdomain);
        _moose_system._kernels[tid].updateActiveKernels(_moose_system._t, _moose_system._dt, cur_subdomain);
      } 

      _moose_system._element_data[tid]->reinitMaterials(_moose_system._materials[tid].getMaterials(cur_subdomain));

      //Stabilizers
      for(stabilizer_it=stabilizer_begin;stabilizer_it!=stabilizer_end;stabilizer_it++)
        stabilizer_it->second->computeTestFunctions();
    
      //Kernels
      KernelIterator kernel_begin = _moose_system._kernels[tid].activeKernelsBegin();
      KernelIterator kernel_end = _moose_system._kernels[tid].activeKernelsEnd();
      KernelIterator kernel_it = kernel_begin;

      for(kernel_it=kernel_begin;kernel_it!=kernel_end;kernel_it++)
      {
        Kernel * kernel = *kernel_it;

        if(kernel->variable() == _ivar)
          kernel->computeOffDiagJacobian(Ke,_jvar);
      }

      for (unsigned int side=0; side<elem->n_sides(); side++)
      {
        std::vector<short int> boundary_ids = _moose_system._mesh->boundary_info->boundary_ids (elem, side);

        if (boundary_ids.size() > 0)
        {
          for (std::vector<short int>::iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
          {
            short int bnd_id = *it;

            BCIterator bc_it = _moose_system._bcs[tid].activeBCsBegin(bnd_id);
            BCIterator bc_end = _moose_system._bcs[tid].activeBCsEnd(bnd_id);

            if(bc_it != bc_end)
            {
              _moose_system.reinitBCs(tid, _soln, elem, side, bnd_id);

              for(; bc_it!=bc_end; ++bc_it)
              {
                BoundaryCondition * bc = *bc_it;

                if(bc->variable() == _ivar)
                  bc->computeJacobianBlock(Ke,_ivar,_jvar);
              }
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

void compute_jacobian_block (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, NonlinearImplicitSystem& sys, unsigned int ivar, unsigned int jvar)
{
  MooseSystem * moose_system = sys.get_equation_systems().parameters.get<MooseSystem *>("moose_system");

  moose_system->computeJacobianBlock (soln, jacobian, precond_system, ivar, jvar);
}

}

void MooseSystem::computeJacobianBlock (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar)
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

  updateAuxVars(soln);
/*
    Threads::parallel_for(ConstElemRange(Moose::mesh->active_local_elements_begin(),
                                         Moose::mesh->active_local_elements_end(),1),
                          ComputeInternalJacobianBlocks(soln, jacobian, precond_system, ivar, jvar));
*/
    
  {
    if(_serialize_solution)
    serializeSolution(*_system->solution);

    if(_has_displaced_mesh)
      updateDisplacedMesh(soln);

    unsigned int tid = 0;
    
    MeshBase::const_element_iterator el = _mesh->active_local_elements_begin();
    const MeshBase::const_element_iterator end_el = _mesh->active_local_elements_end();

    StabilizerIterator stabilizer_begin = _stabilizers[tid].activeStabilizersBegin();
    StabilizerIterator stabilizer_end = _stabilizers[tid].activeStabilizersEnd();
    StabilizerIterator stabilizer_it = stabilizer_begin;

    unsigned int subdomain = std::numeric_limits<unsigned int>::max();

    DofMap & dof_map = precond_system.get_dof_map();
    DenseMatrix<Number> Ke;
    std::vector<unsigned int> dof_indices;

    jacobian.zero();

    for (; el != end_el; ++el)
    {
      const Elem* elem = *el;
      unsigned int cur_subdomain = elem->subdomain_id();

      reinitKernels(tid, soln, elem, NULL, NULL);

      dof_map.dof_indices(elem, dof_indices);

      if(dof_indices.size())
      {
        Ke.resize(dof_indices.size(),dof_indices.size());

        if(cur_subdomain != subdomain)
        {
          subdomain = cur_subdomain;
          subdomainSetup(tid, subdomain);
          _kernels[tid].updateActiveKernels(_t, _dt, cur_subdomain);
        } 

        _element_data[tid]->reinitMaterials(_materials[tid].getMaterials(cur_subdomain));

        //Stabilizers
        for(stabilizer_it=stabilizer_begin;stabilizer_it!=stabilizer_end;stabilizer_it++)
          stabilizer_it->second->computeTestFunctions();
    
        //Kernels
        KernelIterator kernel_begin = _kernels[tid].activeKernelsBegin();
        KernelIterator kernel_end = _kernels[tid].activeKernelsEnd();
        KernelIterator kernel_it = kernel_begin;

        for(kernel_it=kernel_begin;kernel_it!=kernel_end;kernel_it++)
        {
          Kernel * kernel = *kernel_it;

          if(kernel->variable() == ivar)
            kernel->computeOffDiagJacobian(Ke,jvar);
        }

        for (unsigned int side=0; side<elem->n_sides(); side++)
        {
          std::vector<short int> boundary_ids = _mesh->boundary_info->boundary_ids (elem, side);

          if (boundary_ids.size() > 0)
          {
            for (std::vector<short int>::iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
            {
              short int bnd_id = *it;

              BCIterator bc_it = _bcs[tid].activeBCsBegin(bnd_id);
              BCIterator bc_end = _bcs[tid].activeBCsEnd(bnd_id);

              if(bc_it != bc_end)
              {
                reinitBCs(tid, soln, elem, side, bnd_id);

                for(; bc_it!=bc_end; ++bc_it)
                {
                  BoundaryCondition * bc = *bc_it;

                  if(bc->variable() == ivar)
                    bc->computeJacobianBlock(Ke,ivar,jvar);
                }
              }
            }
          }
        }
      
        dof_map.constrain_element_matrix (Ke, dof_indices, false);

        jacobian.add_matrix(Ke, dof_indices);
      }
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
  
    BCIterator bc_it = _bcs[0].activeNodalBCsBegin(boundary_id);
    BCIterator bc_end = _bcs[0].activeNodalBCsEnd(boundary_id);

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
