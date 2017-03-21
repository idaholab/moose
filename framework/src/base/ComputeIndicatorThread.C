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
#include "ComputeIndicatorThread.h"

#include "AuxiliarySystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "Indicator.h"
#include "InternalSideIndicator.h"
#include "SwapBackSentinel.h"

// libmesh includes
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

  std::set<MooseVariable *> needed_moose_vars;
  _indicator_whs.updateVariableDependency(needed_moose_vars, _tid);
  _internal_side_indicators.updateVariableDependency(needed_moose_vars, _tid);
  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);

  std::set<unsigned int> needed_mat_props;
  _indicator_whs.updateMatPropDependency(needed_mat_props, _tid);
  _internal_side_indicators.updateMatPropDependency(needed_mat_props, _tid);
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);

  _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
ComputeIndicatorThread::onElement(const Elem * elem)
{
  for (const auto & it : _aux_sys._elem_vars[_tid])
  {
    MooseVariable * var = it.second;
    var->prepareAux();
  }

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
    for (const auto & it : _aux_sys._elem_vars[_tid])
    {
      MooseVariable * var = it.second;
      var->add(_aux_sys.solution());
    }
  }
}

void
ComputeIndicatorThread::onBoundary(const Elem * /*elem*/,
                                   unsigned int /*side*/,
                                   BoundaryID /*bnd_id*/)
{
}

void
ComputeIndicatorThread::onInternalSide(const Elem * elem, unsigned int side)
{
  if (_finalize) // If finalizing we only do something on the elements
    return;

  // Pointer to the neighbor we are currently working on.
  const Elem * neighbor = elem->neighbor(side);

  // Get the global id of the element and the neighbor
  const dof_id_type elem_id = elem->id(), neighbor_id = neighbor->id();

  if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) ||
      (neighbor->level() < elem->level()))
  {
    for (const auto & it : _aux_sys._elem_vars[_tid])
    {
      MooseVariable * var = it.second;
      var->prepareAux();
    }

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
