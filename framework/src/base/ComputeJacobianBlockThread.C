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

#include "ComputeJacobianBlockThread.h"

#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "TimeDerivative.h"
#include "IntegratedBC.h"
#include "DGKernel.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeJacobianBlockThread::ComputeJacobianBlockThread(FEProblem & fe_problem, libMesh::System & precond_system, SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar) :
    _fe_problem(fe_problem),
    _precond_system(precond_system),
    _nl(_fe_problem.getNonlinearSystem()),
    _mesh(fe_problem.mesh()),
    _jacobian(jacobian),
    _ivar(ivar),
    _jvar(jvar)
{
}

// Splitting Constructor
ComputeJacobianBlockThread::ComputeJacobianBlockThread(ComputeJacobianBlockThread & x, Threads::split /*split*/) :
    _fe_problem(x._fe_problem),
    _precond_system(x._precond_system),
    _nl(x._nl),
    _mesh(x._mesh),
    _jacobian(x._jacobian),
    _ivar(x._ivar),
    _jvar(x._jvar)
{
}

ComputeJacobianBlockThread::~ComputeJacobianBlockThread()
{
}

void
ComputeJacobianBlockThread::operator() (const ConstElemRange & range, bool bypass_threading/*=false*/)
{
  ParallelUniqueId puid;
  _tid = bypass_threading ? 0 : puid.id;

  unsigned int subdomain = std::numeric_limits<unsigned int>::max();

  const DofMap & dof_map = _precond_system.get_dof_map();
  std::vector<unsigned int> dof_indices;

  ConstElemRange::const_iterator range_end = range.end();
  for (ConstElemRange::const_iterator el = range.begin() ; el != range_end; ++el)
  {
    const Elem* elem = *el;
    unsigned int cur_subdomain = elem->subdomain_id();

    dof_map.dof_indices(elem, dof_indices);
    if (dof_indices.size())
    {
      _fe_problem.prepare(elem, _ivar, _jvar, dof_indices, _tid);
      _fe_problem.reinitElem(elem, _tid);

      if (cur_subdomain != subdomain)
      {
        subdomain = cur_subdomain;
        _fe_problem.subdomainSetup(subdomain, _tid);
        _nl._kernels[_tid].updateActiveKernels(cur_subdomain);
      }

      _fe_problem.reinitMaterials(cur_subdomain, _tid);

      //Kernels
      std::vector<Kernel *> kernels = _nl._kernels[_tid].active();
      for (std::vector<Kernel *>::const_iterator it = kernels.begin(); it != kernels.end(); it++)
      {
        Kernel * kernel = *it;
        if (kernel->variable().index() == _ivar)
        {
          kernel->subProblem().prepareShapes(_jvar, _tid);
          kernel->computeOffDiagJacobian(_jvar);
        }
      }

      _fe_problem.swapBackMaterials(_tid);

      for (unsigned int side = 0; side < elem->n_sides(); side++)
      {
        std::vector<BoundaryID> boundary_ids = _mesh.boundaryIDs(elem, side);

        if (boundary_ids.size() > 0)
        {
          for (std::vector<BoundaryID>::iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
          {
            BoundaryID bnd_id = *it;

            std::vector<IntegratedBC *> bcs = _nl._bcs[_tid].activeIntegrated(bnd_id);
            if (bcs.size() > 0)
            {
              _fe_problem.prepareFace(elem, _tid);
              _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);
              _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
              _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

              for (std::vector<IntegratedBC *>::iterator it = bcs.begin(); it != bcs.end(); ++it)
              {
                IntegratedBC * bc = *it;
                if (bc->variable().index() == _ivar)
                {
                  if (bc->shouldApply())
                  {
                    bc->subProblem().prepareFaceShapes(_jvar, _tid);
                    bc->computeJacobianBlock(_jvar);
                  }
                }
              }

              _fe_problem.swapBackMaterialsFace(_tid);
            }
          }
        }

        if (elem->neighbor(side) != NULL)
        {
          // on internal edge
          // Pointer to the neighbor we are currently working on.
          const Elem * neighbor = elem->neighbor(side);

          // Get the global id of the element and the neighbor
          const unsigned int elem_id = elem->id();
          const unsigned int neighbor_id = neighbor->id();

          if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) || (neighbor->level() < elem->level()))
          {
            std::vector<DGKernel *> dgks = _nl._dg_kernels[_tid].active();
            if (dgks.size() > 0)
            {
              _fe_problem.prepareFace(elem, _tid);
              _fe_problem.reinitNeighbor(elem, side, _tid);

              _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
              _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

              for (std::vector<DGKernel *>::iterator it = dgks.begin(); it != dgks.end(); ++it)
              {
                DGKernel * dg = *it;
                if (dg->variable().index() == _ivar)
                {
                  dg->subProblem().prepareFaceShapes(_jvar, _tid);
                  dg->subProblem().prepareNeighborShapes(_jvar, _tid);
                  dg->computeOffDiagJacobian(_jvar);
                }
              }

              _fe_problem.swapBackMaterialsFace(_tid);
              _fe_problem.swapBackMaterialsNeighbor(_tid);

              std::vector<unsigned int> neighbor_dof_indices;
              dof_map.dof_indices(neighbor, neighbor_dof_indices);
              {
                Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
                _fe_problem.addJacobianNeighbor(_jacobian, _ivar, _jvar, dof_map, dof_indices, neighbor_dof_indices, _tid);
              }
            }
          }
        }
      }

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.addJacobianBlock(_jacobian, _ivar, _jvar, dof_map, dof_indices, _tid);
      }
    }
  }
}

void
ComputeJacobianBlockThread::join(const ComputeJacobianBlockThread & /*y*/)
{
}
