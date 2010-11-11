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
#include "MaterialFactory.h"
#include "BoundaryCondition.h"
#include "DGKernel.h"
#include "ParallelUniqueId.h"
#include "MooseSystem.h"
#include "DofData.h"
#include "ElementData.h"
#include "ComputeBase.h"

//libMesh includes
#include "numeric_vector.h"
#include "dense_vector.h"
#include "petsc_matrix.h"
#include "dof_map.h"
#include "mesh.h"
#include "boundary_info.h"
#include "elem_range.h"

#include <vector>

class ComputeInternalResiduals : public ComputeBase
{
public:
  ComputeInternalResiduals(MooseSystem &sys, const NumericVector<Number>& in_soln, NumericVector<Number>& in_residual) :
    ComputeBase(sys),
    _soln(in_soln),
    _residual(in_residual)
  {}

  // Splitting Constructor
  ComputeInternalResiduals(ComputeInternalResiduals & x, Threads::split) :
    ComputeBase(x._moose_system),
    _soln(x._soln),
    _residual(x._residual)
  {
  }

  virtual void pre()
  {
    _moose_system._dg_kernels[_tid].updateActiveDGKernels(_moose_system._t, _moose_system._dt);
  }

  virtual void preElement(const Elem *elem)
  {
    _re.zero();
    _moose_system.reinitKernels(_tid, _soln, elem, &_re);
  }

  virtual void onElement(const Elem *elem)
  {
    unsigned int cur_subdomain = elem->subdomain_id();
    _moose_system._element_data[_tid]->reinitMaterials(_moose_system._materials[_tid].getMaterials(cur_subdomain));

    //Stabilizers
    StabilizerIterator stabilizer_begin = _moose_system._stabilizers[_tid].activeStabilizersBegin();
    StabilizerIterator stabilizer_end = _moose_system._stabilizers[_tid].activeStabilizersEnd();
    StabilizerIterator stabilizer_it = stabilizer_begin;

    for(stabilizer_it=stabilizer_begin;stabilizer_it!=stabilizer_end;stabilizer_it++)
      stabilizer_it->second->computeTestFunctions();

    //Kernels
    KernelIterator kernel_begin = _moose_system._kernels[_tid].activeKernelsBegin();
    KernelIterator kernel_end = _moose_system._kernels[_tid].activeKernelsEnd();
    KernelIterator kernel_it = kernel_begin;

    for(kernel_it=kernel_begin;kernel_it!=kernel_end;++kernel_it)
      (*kernel_it)->computeResidual();
  }

  virtual void onDomainChanged(short int subdomain)
  {
    _moose_system.subdomainSetup(_tid, subdomain);
    _moose_system._kernels[_tid].updateActiveKernels(_moose_system._t, _moose_system._dt, subdomain);
  }

  virtual void postElement(const Elem * /*elem*/)
  {
    _moose_system._dof_map->constrain_element_vector (_re, _moose_system._dof_data[_tid]._dof_indices, false);
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      _residual.add_vector(_re, _moose_system._dof_data[_tid]._dof_indices);
    }
  }

  virtual void onBoundary(const Elem *elem, unsigned int side, short int bnd_id)
  {
    BCIterator bc_it = _moose_system._bcs[_tid].activeBCsBegin(bnd_id);
    BCIterator bc_end = _moose_system._bcs[_tid].activeBCsEnd(bnd_id);

    if(bc_it != bc_end)
    {
      _moose_system.reinitBCs(_tid, _soln, elem, side, bnd_id);

      for(; bc_it!=bc_end; ++bc_it)
        (*bc_it)->computeResidual();
    }
  }

  virtual void onInternalSide(const Elem *elem, unsigned int side)
  {
    // Pointer to the neighbor we are currently working on.
    const Elem * neighbor = elem->neighbor(side);

    // Get the global id of the element and the neighbor
    const unsigned int elem_id = elem->id();
    const unsigned int neighbor_id = neighbor->id();

    if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) || (neighbor->level() < elem->level()))
    {
      DGKernelIterator dg_it = _moose_system._dg_kernels[_tid].activeDGKernelsBegin();
      DGKernelIterator dg_end = _moose_system._dg_kernels[_tid].activeDGKernelsEnd();

      if (dg_it!=dg_end)
      {
        DenseVector<Number> neighbor_Re;
        neighbor_Re.zero();

        _moose_system.reinitDGKernels(_tid, _soln, elem, side, neighbor, &neighbor_Re);

        for(; dg_it!=dg_end; ++dg_it)
          (*dg_it)->computeResidual();

        _moose_system._dof_map->constrain_element_vector (neighbor_Re, _moose_system._neighbor_dof_data[_tid]._dof_indices, false);
        {
          Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
          _residual.add_vector(neighbor_Re, _moose_system._neighbor_dof_data[_tid]._dof_indices);
        }
      }
    }
  }

  void join(const ComputeInternalResiduals & /*y*/)
  {
  }

protected:
  const NumericVector<Number>& _soln;
  NumericVector<Number>& _residual;

  DenseVector<Number> _re;
};


namespace Moose
{
  void compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual, NonlinearImplicitSystem& sys)
  {
    Moose::perf_log.push("compute_residual()","Solve");

    MooseSystem * moose_system = sys.get_equation_systems().parameters.get<MooseSystem *>("moose_system");
    mooseAssert(moose_system != NULL, "Internal pointer to MooseSystem was not set");
    moose_system->computeResidual(soln, residual);
    
    Moose::perf_log.pop("compute_residual()","Solve");
  }
}

void MooseSystem::computeResidual (const NumericVector<Number>& soln, NumericVector<Number>& residual)
{
  if (needPostprocessorsForResiduals())
    computePostprocessors(*(getNonlinearSystem()->current_local_solution));
  computeTimeDeriv(soln);
  computeResidualInternal(soln, residual);
  finishResidual(residual);
}

void MooseSystem::computeResidualInternal (const NumericVector<Number>& soln, NumericVector<Number>& residual)
{
  residual.zero();

  if(_serialize_solution)
    serializeSolution(soln);

  if(_has_displaced_mesh)
    updateDisplacedMesh(soln);

  updateAuxVars(soln);

  ComputeInternalResiduals cr(*this, soln, residual);
  Threads::parallel_reduce(*getActiveLocalElementRange(), cr);

  residual.close();

  if(needResidualCopy())
    *_residual_copy = residual;

  //Dirichlet BCs
  std::vector<unsigned int> nodes;
  std::vector<short int> ids;

  _mesh->boundary_info->build_node_list(nodes, ids);

  const std::set<short int> & boundary_ids = _mesh->boundary_info->get_boundary_ids();

  for(std::set<short int>::const_iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
  {
    short int id = *it;
    
    BCIterator bc_it = _bcs[0].activeNodalBCsBegin(id);
    BCIterator bc_end = _bcs[0].activeNodalBCsEnd(id);

    for(; bc_it != bc_end; ++bc_it)
      (*bc_it)->setup();
  }

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
        if (bc_it != bc_end)
          reinitBCs(0, soln, node, boundary_id, residual);

        for(; bc_it != bc_end; ++bc_it)
          (*bc_it)->computeAndStoreResidual();
      }
    }
  }
  
  residual.close();
}

