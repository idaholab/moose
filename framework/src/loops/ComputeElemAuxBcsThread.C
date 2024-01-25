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
    FEProblemBase & fe_problem,
    const MooseObjectWarehouse<AuxKernelType> & storage,
    bool need_materials)
  : _fe_problem(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _storage(storage),
    _need_materials(need_materials)
{
}

// Splitting Constructor
template <typename AuxKernelType>
ComputeElemAuxBcsThread<AuxKernelType>::ComputeElemAuxBcsThread(ComputeElemAuxBcsThread & x,
                                                                Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _aux_sys(x._aux_sys),
    _storage(x._storage),
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

  printGeneralExecutionInformation();

  for (const auto & belem : range)
  {
    const Elem * elem = belem->_elem;
    unsigned short int side = belem->_side;
    BoundaryID boundary_id = belem->_bnd_id;
    SubdomainID last_did = Elem::invalid_subdomain_id;

    // need to update the boundary ID in assembly
    _fe_problem.setCurrentBoundaryID(boundary_id, _tid);

    if (elem->processor_id() == _fe_problem.processor_id())
    {
      // Locate the AuxKernel objects for the current BoundaryID
      const auto iter = boundary_kernels.find(boundary_id);

      if (iter != boundary_kernels.end() && !(iter->second.empty()))
      {
        printBoundaryExecutionInformation(boundary_id, iter->second);
        auto did = elem->subdomain_id();
        if (did != last_did)
        {
          _fe_problem.subdomainSetup(did, _tid);
          last_did = did;
        }
        _fe_problem.setCurrentSubdomainID(elem, _tid);
        _fe_problem.prepare(elem, _tid);
        _fe_problem.reinitElemFace(elem, side, boundary_id, _tid);

        const Elem * lower_d_elem = _fe_problem.mesh().getLowerDElem(elem, side);
        if (lower_d_elem)
          _fe_problem.reinitLowerDElem(lower_d_elem, _tid);

        const Elem * neighbor = elem->neighbor_ptr(side);

        // The last check here is absolutely necessary otherwise we will attempt to evaluate
        // neighbor materials on neighbor elements that aren't evaluable, e.g. don't have algebraic
        // ghosting
        bool compute_interface =
            neighbor && neighbor->active() &&
            _fe_problem.getInterfaceMaterialsWarehouse().hasActiveBoundaryObjects(boundary_id,
                                                                                  _tid);

        // Set up Sentinel class so that, even if reinitMaterialsFace() throws, we
        // still remember to swap back during stack unwinding.
        SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);
        SwapBackSentinel neighbor_sentinel(
            _fe_problem, &FEProblem::swapBackMaterialsNeighbor, _tid, compute_interface);

        if (_need_materials)
        {
          std::set<unsigned int> needed_mat_props;
          for (const auto & aux : iter->second)
          {
            const std::set<unsigned int> & mp_deps = aux->getMatPropDependencies();
            needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
          }
          _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);

          _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);

          _fe_problem.reinitMaterialsBoundary(boundary_id, _tid);

          if (compute_interface)
          {
            _fe_problem.reinitNeighbor(elem, side, _tid);
            _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);
            _fe_problem.reinitMaterialsInterface(boundary_id, _tid);
          }
        }

        for (const auto & aux : iter->second)
        {
          aux->compute();
          aux->variable().insert(_aux_sys.solution());
        }

        if (_need_materials)
          _fe_problem.clearActiveMaterialProperties(_tid);
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
ComputeElemAuxBcsThread<AuxKernelType>::printGeneralExecutionInformation() const
{
  if (_fe_problem.shouldPrintExecution(_tid) && _storage.hasActiveObjects())
  {
    const auto & console = _fe_problem.console();
    const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
    console << "[DBG] Executing boundary restricted auxkernels on boundary elements on "
            << execute_on << std::endl;
  }
}

template <typename AuxKernelType>
void
ComputeElemAuxBcsThread<AuxKernelType>::printBoundaryExecutionInformation(
    unsigned int boundary_id, const std::vector<std::shared_ptr<AuxKernelType>> & kernels) const
{
  if (!_fe_problem.shouldPrintExecution(_tid) || !_storage.hasActiveObjects() ||
      _boundaries_exec_printed.count(boundary_id))
    return;

  const auto & console = _fe_problem.console();
  console << "[DBG] Ordering on boundary " << boundary_id << std::endl;
  std::vector<MooseObject *> objs_ptrs;
  for (auto & kernel_ptr : kernels)
    if (kernel_ptr->hasBoundary(boundary_id))
      objs_ptrs.push_back(dynamic_cast<MooseObject *>(kernel_ptr.get()));
  std::string list_kernels = ConsoleUtils::mooseObjectVectorToString(objs_ptrs);
  console << ConsoleUtils::formatString(list_kernels, "[DBG]") << std::endl;
  _boundaries_exec_printed.insert(boundary_id);
}

template class ComputeElemAuxBcsThread<AuxKernel>;
template class ComputeElemAuxBcsThread<VectorAuxKernel>;
template class ComputeElemAuxBcsThread<ArrayAuxKernel>;
