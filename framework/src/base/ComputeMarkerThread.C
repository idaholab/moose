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
#include "ComputeMarkerThread.h"

#include "AuxiliarySystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "Marker.h"

// libmesh includes
#include "threads.h"

ComputeMarkerThread::ComputeMarkerThread(FEProblem & fe_problem,
                                               AuxiliarySystem & sys,
                                               std::vector<MarkerWarehouse> & marker_whs) :
    ThreadedElementLoop<ConstElemRange>(fe_problem, sys),
    _fe_problem(fe_problem),
    _aux_sys(sys),
    _marker_whs(marker_whs)
{
}

// Splitting Constructor
ComputeMarkerThread::ComputeMarkerThread(ComputeMarkerThread & x, Threads::split split) :
    ThreadedElementLoop<ConstElemRange>(x, split),
    _fe_problem(x._fe_problem),
    _aux_sys(x._aux_sys),
    _marker_whs(x._marker_whs)
{
}

ComputeMarkerThread::~ComputeMarkerThread()
{
}

void
ComputeMarkerThread::subdomainChanged()
{
  _fe_problem.subdomainSetup(_subdomain, _tid);
  _marker_whs[_tid].updateActiveMarkers(_subdomain);

  const std::vector<Marker *> & markers = _marker_whs[_tid].active();
  for (std::vector<Marker *>::const_iterator it = markers.begin(); it != markers.end(); ++it)
    (*it)->subdomainSetup();

  std::set<MooseVariable *> needed_moose_vars;

  for (std::vector<Marker *>::const_iterator it = markers.begin(); it != markers.end(); ++it)
  {
    const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
    needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
  }

  for (std::map<std::string, MooseVariable *>::iterator it = _aux_sys._elem_vars[_tid].begin(); it != _aux_sys._elem_vars[_tid].end(); ++it)
  {
    MooseVariable * var = it->second;
    var->prepare_aux();
  }

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
ComputeMarkerThread::onElement(const Elem *elem)
{
  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);

  _fe_problem.reinitMaterials(_subdomain, _tid);

  const std::vector<Marker *> & markers = _marker_whs[_tid].active();
  for (std::vector<Marker *>::const_iterator it = markers.begin(); it != markers.end(); ++it)
    (*it)->computeMarker();

  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (std::map<std::string, MooseVariable *>::iterator it = _aux_sys._elem_vars[_tid].begin(); it != _aux_sys._elem_vars[_tid].end(); ++it)
    {
      MooseVariable * var = it->second;
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
