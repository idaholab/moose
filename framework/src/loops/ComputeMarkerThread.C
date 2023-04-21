//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ComputeMarkerThread.h"
#include "AuxiliarySystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "Marker.h"
#include "MooseVariableFE.h"
#include "SwapBackSentinel.h"

#include "libmesh/threads.h"

ComputeMarkerThread::ComputeMarkerThread(FEProblemBase & fe_problem)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _fe_problem(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _marker_whs(_fe_problem.getMarkerWarehouse())
{
}

// Splitting Constructor
ComputeMarkerThread::ComputeMarkerThread(ComputeMarkerThread & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _fe_problem(x._fe_problem),
    _aux_sys(x._aux_sys),
    _marker_whs(x._marker_whs)
{
}

ComputeMarkerThread::~ComputeMarkerThread() {}

void
ComputeMarkerThread::subdomainChanged()
{
  _fe_problem.subdomainSetup(_subdomain, _tid);
  _marker_whs.subdomainSetup(_tid);

  std::set<MooseVariableFEBase *> needed_moose_vars;
  _marker_whs.updateVariableDependency(needed_moose_vars, _tid);

  for (auto * var : _aux_sys._elem_vars[_tid])
    var->prepareAux();

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
ComputeMarkerThread::onElement(const Elem * elem)
{
  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);

  // Set up Sentinel class so that, even if reinitMaterials() throws, we
  // still remember to swap back during stack unwinding.
  SwapBackSentinel sentinel(_fe_problem,
                            &FEProblem::swapBackMaterials,
                            _tid,
                            _fe_problem.hasActiveMaterialProperties(_tid));

  _fe_problem.reinitMaterials(_subdomain, _tid);

  if (_marker_whs.hasActiveBlockObjects(_subdomain, _tid))
  {
    const std::vector<std::shared_ptr<Marker>> & markers =
        _marker_whs.getActiveBlockObjects(_subdomain, _tid);
    for (const auto & marker : markers)
      marker->computeMarker();
  }

  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (auto * var : _aux_sys._elem_vars[_tid])
      var->insert(_aux_sys.solution());
  }
}

void
ComputeMarkerThread::onBoundary(const Elem * /*elem*/,
                                unsigned int /*side*/,
                                BoundaryID /*bnd_id*/,
                                const Elem * /*lower_d_elem = nullptr*/)
{
}

void
ComputeMarkerThread::onInternalSide(const Elem * /*elem*/, unsigned int /*side*/)
{
}

void
ComputeMarkerThread::postElement(const Elem * /*elem*/)
{
}

void
ComputeMarkerThread::post()
{
  _fe_problem.clearActiveElementalMooseVariables(_tid);
}

void
ComputeMarkerThread::join(const ComputeMarkerThread & /*y*/)
{
}

void
ComputeMarkerThread::printGeneralExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid))
    return;
  const auto & console = _fe_problem.console();
  const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
  console << "[DBG] Beginning elemental loop to compute Markers on " << execute_on << std::endl;
}

void
ComputeMarkerThread::printBlockExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid) || _blocks_exec_printed.count(_subdomain) ||
      !_marker_whs.hasActiveBlockObjects(_subdomain, _tid))
    return;

  const auto & console = _fe_problem.console();
  const auto & markers = _marker_whs.getActiveBlockObjects(_subdomain, _tid);
  console << "[DBG] Execution order on block: " << _subdomain << std::endl;
  printExecutionOrdering<Marker>(markers, false);
  _blocks_exec_printed.insert(_subdomain);
}
