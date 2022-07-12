//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ComputeElemAuxBcsThread.h"
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "Assembly.h"
#include "AuxKernel.h"
#include "SwapBackSentinel.h"

#include "libmesh/threads.h"

template <typename AuxKernelType>
ComputeElemAuxBcsThread<AuxKernelType>::ComputeElemAuxBcsThread(
    FEProblemBase & problem,
    const MooseObjectWarehouse<AuxKernelType> & storage,
    const std::vector<std::vector<MooseVariableFEBase *>> & vars,
    bool need_materials)
  : _problem(problem),
    _aux_sys(problem.getAuxiliarySystem()),
    _storage(storage),
    _aux_vars(vars),
    _need_materials(need_materials)
{
}

// Splitting Constructor
template <typename AuxKernelType>
ComputeElemAuxBcsThread<AuxKernelType>::ComputeElemAuxBcsThread(ComputeElemAuxBcsThread & x,
                                                                Threads::split /*split*/)
  : _problem(x._problem),
    _aux_sys(x._aux_sys),
    _storage(x._storage),
    _aux_vars(x._aux_vars),
    _need_materials(x._need_materials)
{
}

template <typename AuxKernelType>
void
ComputeElemAuxBcsThread<AuxKernelType>::operator()(const ConstBndElemRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  // Reference to all boundary restricted AuxKernels for the current thread
  const auto & boundary_kernels = _storage.getActiveBoundaryObjects(_tid);

  printExecutionInformation();

  for (const auto & belem : range)
  {
    const Elem * elem = belem->_elem;
    unsigned short int side = belem->_side;
    BoundaryID boundary_id = belem->_bnd_id;
    SubdomainID last_did = Elem::invalid_subdomain_id;

    // need to update the boundary ID in assembly
    _problem.setCurrentBoundaryID(boundary_id, _tid);

    if (elem->processor_id() == _problem.processor_id())
    {
      // prepare variables
      for (auto * var : _aux_vars[_tid])
        var->prepareAux();

      // Locate the AuxKernel objects for the current BoundaryID
      const auto iter = boundary_kernels.find(boundary_id);

      if (iter != boundary_kernels.end() && !(iter->second.empty()))
      {
        auto did = elem->subdomain_id();
        if (did != last_did)
        {
          _problem.subdomainSetup(did, _tid);
          last_did = did;
        }
        _problem.setCurrentSubdomainID(elem, _tid);
        _problem.prepare(elem, _tid);
        _problem.reinitElemFace(elem, side, boundary_id, _tid);

        const Elem * lower_d_elem = _problem.mesh().getLowerDElem(elem, side);
        if (lower_d_elem)
          _problem.reinitLowerDElem(lower_d_elem, _tid);

        const Elem * neighbor = elem->neighbor_ptr(side);

        // The last check here is absolutely necessary otherwise we will attempt to evaluate
        // neighbor materials on neighbor elements that aren't evaluable, e.g. don't have algebraic
        // ghosting
        bool compute_interface =
            neighbor && neighbor->active() &&
            _problem.getInterfaceMaterialsWarehouse().hasActiveBoundaryObjects(boundary_id, _tid);

        // Set up Sentinel class so that, even if reinitMaterialsFace() throws, we
        // still remember to swap back during stack unwinding.
        SwapBackSentinel sentinel(_problem, &FEProblem::swapBackMaterialsFace, _tid);
        SwapBackSentinel neighbor_sentinel(
            _problem, &FEProblem::swapBackMaterialsNeighbor, _tid, compute_interface);

        if (_need_materials)
        {
          std::set<unsigned int> needed_mat_props;
          for (const auto & aux : iter->second)
          {
            const std::set<unsigned int> & mp_deps = aux->getMatPropDependencies();
            needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
          }
          _problem.setActiveMaterialProperties(needed_mat_props, _tid);

          _problem.reinitMaterialsFace(elem->subdomain_id(), _tid);

          _problem.reinitMaterialsBoundary(boundary_id, _tid);

          if (compute_interface)
          {
            _problem.reinitNeighbor(elem, side, _tid);
            _problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);
            _problem.reinitMaterialsInterface(boundary_id, _tid);
          }
        }

        for (const auto & aux : iter->second)
          aux->compute();

        if (_need_materials)
          _problem.clearActiveMaterialProperties(_tid);
      }

      // update the solution vector
      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        for (auto * var : _aux_vars[_tid])
          var->insert(_aux_sys.solution());
      }
    }
  }
}

template <typename AuxKernelType>
void
ComputeElemAuxBcsThread<AuxKernelType>::join(const ComputeElemAuxBcsThread & /*y*/)
{
}

template <typename AuxKernelType>
void
ComputeElemAuxBcsThread<AuxKernelType>::printExecutionInformation() const
{
  if (_problem.shouldPrintExecution())
  {
    auto console = _problem.console();
    console << "Executing boundary restricted auxkernels on boundary elements " << std::endl;
    console << "Ordering" << std::endl;
    console << _storage.activeObjectsToString() << std::endl;
    console << "They are executed in that order on the sides these kernels are defined on." << std::endl;
  }
}

template class ComputeElemAuxBcsThread<AuxKernel>;
template class ComputeElemAuxBcsThread<VectorAuxKernel>;
template class ComputeElemAuxBcsThread<ArrayAuxKernel>;
