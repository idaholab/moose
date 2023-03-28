//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeIndicatorThread.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "Indicator.h"
#include "InternalSideIndicator.h"
#include "MooseVariableFE.h"
#include "Problem.h"
#include "SwapBackSentinel.h"
// For dynamic casting to Coupleable
#include "Material.h"
#include "InterfaceMaterial.h"

#include "libmesh/threads.h"

ComputeIndicatorThread::ComputeIndicatorThread(FEProblemBase & fe_problem, bool finalize)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _fe_problem(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _indicator_whs(_fe_problem.getIndicatorWarehouse()),
    _internal_side_indicators(_fe_problem.getInternalSideIndicatorWarehouse()),
    _finalize(finalize)
{
}

// Splitting Constructor
ComputeIndicatorThread::ComputeIndicatorThread(ComputeIndicatorThread & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _fe_problem(x._fe_problem),
    _aux_sys(x._aux_sys),
    _indicator_whs(x._indicator_whs),
    _internal_side_indicators(x._internal_side_indicators),
    _finalize(x._finalize)
{
}

ComputeIndicatorThread::~ComputeIndicatorThread() {}

void
ComputeIndicatorThread::subdomainChanged()
{
  _fe_problem.subdomainSetup(_subdomain, _tid);

  _indicator_whs.subdomainSetup(_tid);
  _internal_side_indicators.subdomainSetup(_tid);

  std::set<MooseVariableFEBase *> needed_moose_vars;
  _indicator_whs.updateVariableDependency(needed_moose_vars, _tid);
  _internal_side_indicators.updateVariableDependency(needed_moose_vars, _tid);
  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);

  // Update variable coupleable vector tags
  std::set<TagID> needed_var_vector_tags;
  _indicator_whs.updateBlockFEVariableCoupledVectorTagDependency(
      _subdomain, needed_var_vector_tags, _tid);
  _internal_side_indicators.updateBlockFEVariableCoupledVectorTagDependency(
      _subdomain, needed_var_vector_tags, _tid);
  _fe_problem.getMaterialWarehouse().updateBlockFEVariableCoupledVectorTagDependency(
      _subdomain, needed_var_vector_tags, _tid);
  _fe_problem.setActiveFEVariableCoupleableVectorTags(needed_var_vector_tags, _tid);

  std::set<unsigned int> needed_mat_props;
  _indicator_whs.updateMatPropDependency(needed_mat_props, _tid);
  _internal_side_indicators.updateMatPropDependency(needed_mat_props, _tid);
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);

  _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
ComputeIndicatorThread::onElement(const Elem * elem)
{
  for (auto * var : _aux_sys._elem_vars[_tid])
    var->prepareAux();

  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);

  // Set up Sentinel class so that, even if reinitMaterials() throws, we
  // still remember to swap back during stack unwinding.
  SwapBackSentinel sentinel(_fe_problem, &FEProblemBase::swapBackMaterials, _tid);

  _fe_problem.reinitMaterials(_subdomain, _tid);

  // Compute
  if (!_finalize)
  {
    if (_indicator_whs.hasActiveBlockObjects(_subdomain, _tid))
    {
      const std::vector<std::shared_ptr<Indicator>> & indicators =
          _indicator_whs.getActiveBlockObjects(_subdomain, _tid);
      for (const auto & indicator : indicators)
        indicator->computeIndicator();
    }
  }

  // Finalize
  else
  {
    if (_indicator_whs.hasActiveBlockObjects(_subdomain, _tid))
    {
      const std::vector<std::shared_ptr<Indicator>> & indicators =
          _indicator_whs.getActiveBlockObjects(_subdomain, _tid);
      for (const auto & indicator : indicators)
        indicator->finalize();
    }

    if (_internal_side_indicators.hasActiveBlockObjects(_subdomain, _tid))
    {
      const std::vector<std::shared_ptr<InternalSideIndicator>> & internal_indicators =
          _internal_side_indicators.getActiveBlockObjects(_subdomain, _tid);
      for (const auto & internal_indicator : internal_indicators)
        internal_indicator->finalize();
    }
  }

  if (!_finalize) // During finalize the Indicators should be setting values in the vectors manually
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (auto * var : _aux_sys._elem_vars[_tid])
      var->add(_aux_sys.solution());
  }
}

void
ComputeIndicatorThread::onBoundary(const Elem * /*elem*/,
                                   unsigned int /*side*/,
                                   BoundaryID /*bnd_id*/,
                                   const Elem * /*lower_d_elem = nullptr*/)
{
}

void
ComputeIndicatorThread::onInternalSide(const Elem * elem, unsigned int side)
{
  if (_finalize) // If finalizing we only do something on the elements
    return;

  // Pointer to the neighbor we are currently working on.
  const Elem * neighbor = elem->neighbor_ptr(side);

  for (auto * var : _aux_sys._elem_vars[_tid])
    var->prepareAux();

  SubdomainID block_id = elem->subdomain_id();
  if (_internal_side_indicators.hasActiveBlockObjects(block_id, _tid))
  {
    _fe_problem.reinitNeighbor(elem, side, _tid);

    // Set up Sentinels so that, even if one of the reinitMaterialsXXX() calls throws, we
    // still remember to swap back during stack unwinding.
    SwapBackSentinel face_sentinel(_fe_problem, &FEProblemBase::swapBackMaterialsFace, _tid);
    _fe_problem.reinitMaterialsFace(block_id, _tid);

    SwapBackSentinel neighbor_sentinel(
        _fe_problem, &FEProblemBase::swapBackMaterialsNeighbor, _tid);
    _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

    const std::vector<std::shared_ptr<InternalSideIndicator>> & indicators =
        _internal_side_indicators.getActiveBlockObjects(block_id, _tid);
    for (const auto & indicator : indicators)
      indicator->computeIndicator();
  }
}

void
ComputeIndicatorThread::postElement(const Elem * /*elem*/)
{
}

void
ComputeIndicatorThread::post()
{
  _fe_problem.clearActiveElementalMooseVariables(_tid);
  _fe_problem.clearActiveMaterialProperties(_tid);
}

void
ComputeIndicatorThread::join(const ComputeIndicatorThread & /*y*/)
{
}

void
ComputeIndicatorThread::printGeneralExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid))
    return;

  const auto & console = _fe_problem.console();
  const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
  if (!_finalize)
    console << "[DBG] Executing indicators on elements then on internal sides on " << execute_on
            << std::endl;
  else
    console << "[DBG] Finalizing indicator loop" << std::endl;
}

void
ComputeIndicatorThread::printBlockExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid) || _blocks_exec_printed.count(_subdomain))
    return;

  const auto & console = _fe_problem.console();
  if (_indicator_whs.hasActiveBlockObjects(_subdomain, _tid))
  {
    const auto & indicators = _indicator_whs.getActiveBlockObjects(_subdomain, _tid);
    console << "[DBG] Ordering of element indicators on block " << _subdomain << std::endl;
    printExecutionOrdering<Indicator>(indicators, false);
  }
  if (_internal_side_indicators.hasActiveBlockObjects(_subdomain, _tid))
  {
    const auto & indicators = _internal_side_indicators.getActiveBlockObjects(_subdomain, _tid);
    console << "[DBG] Ordering of element internal sides indicators on block " << _subdomain
            << std::endl;
    printExecutionOrdering<InternalSideIndicator>(indicators, false);
  }
  _blocks_exec_printed.insert(_subdomain);
}
