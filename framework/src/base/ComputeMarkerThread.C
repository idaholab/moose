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

// MOOSE includes
#include "ComputeMarkerThread.h"
#include "AuxiliarySystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "Marker.h"
#include "SwapBackSentinel.h"

// libmesh includes
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

  std::set<MooseVariable *> needed_moose_vars;
  _marker_whs.updateVariableDependency(needed_moose_vars, _tid);

  for (const auto & it : _aux_sys._elem_vars[_tid])
  {
    MooseVariable * var = it.second;
    var->prepareAux();
  }

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
  SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterials, _tid);

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
    for (const auto & it : _aux_sys._elem_vars[_tid])
    {
      MooseVariable * var = it.second;
      var->insert(_aux_sys.solution());
    }
  }
}

void
ComputeMarkerThread::onBoundary(const Elem * /*elem*/, unsigned int /*side*/, BoundaryID /*bnd_id*/)
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
