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
    :_moose_system(sys),
     _soln(in_soln),
     residual(in_residual)
  {}

  void operator() (const ConstElemRange & range) const
  {
    ParallelUniqueId puid;

    THREAD_ID tid = puid.id;

    DenseVector<Number> Re;

    ConstElemRange::const_iterator el = range.begin();

    _moose_system._dg_kernels[tid].updateActiveDGKernels(_moose_system._t, _moose_system._dt);

    StabilizerIterator stabilizer_begin = _moose_system._stabilizers[tid].activeStabilizersBegin();
    StabilizerIterator stabilizer_end = _moose_system._stabilizers[tid].activeStabilizersEnd();
    StabilizerIterator stabilizer_it = stabilizer_begin;

    subdomain_id_type subdomain = std::numeric_limits<subdomain_id_type>::max();

    for (el = range.begin() ; el != range.end(); ++el)
    {
      const Elem* elem = *el;
      subdomain_id_type cur_subdomain = elem->subdomain_id();

      Re.zero();

      _moose_system.reinitKernels(tid, _soln, elem, &Re);

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

      for(kernel_it=kernel_begin;kernel_it!=kernel_end;++kernel_it)
        (*kernel_it)->computeResidual();

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
                (*bc_it)->computeResidual();
            }
          }
        }

        if (elem->neighbor(side) != NULL)
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
            DGKernelIterator dg_it = _moose_system._dg_kernels[tid].activeDGKernelsBegin();
            DGKernelIterator dg_end = _moose_system._dg_kernels[tid].activeDGKernelsEnd();

            if (dg_it!=dg_end)
            {
              DenseVector<Number> neighbor_Re;
              neighbor_Re.zero();

              _moose_system.reinitDGKernels(tid, _soln, elem, side, neighbor, &neighbor_Re);

              for(; dg_it!=dg_end; ++dg_it)
                (*dg_it)->computeResidual();

              _moose_system._dof_map->constrain_element_vector (neighbor_Re, _moose_system._neighbor_dof_data[tid]._dof_indices, false);
              {
                Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
                residual.add_vector(neighbor_Re, _moose_system._neighbor_dof_data[tid]._dof_indices);
              }
            }
          }
        }
      }

      _moose_system._dof_map->constrain_element_vector (Re, _moose_system._dof_data[tid]._dof_indices, false);
      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        residual.add_vector(Re, _moose_system._dof_data[tid]._dof_indices);
      }
    }
  }

protected:
  MooseSystem &_moose_system;
  const NumericVector<Number>& _soln;
  NumericVector<Number>& residual;
};


namespace Moose
{
  void compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual, NonlinearImplicitSystem& sys)
  {
    MooseSystem * moose_system = sys.get_equation_systems().parameters.get<MooseSystem *>("moose_system");
    mooseAssert(moose_system != NULL, "Internal pointer to MooseSystem was not set");
    if (moose_system->needPostprocessorsForResiduals())
      moose_system->compute_postprocessors(*(moose_system->getNonlinearSystem()->current_local_solution));
    moose_system->compute_residual(soln, residual);
  }
}
      


void MooseSystem::compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual)
{
  Moose::perf_log.push("compute_residual()","Solve");

  residual.zero();

  if(_has_displaced_mesh)
    updateDisplacedMesh(soln);

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


//    Parallel::barrier();
//    residual.print();

  residual.close();

  Moose::perf_log.pop("compute_residual()","Solve");
}

