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

void
ComputeMarkerThread::onElement(const Elem *elem)
{

  for (std::map<std::string, MooseVariable *>::iterator it = _aux_sys._elem_vars[_tid].begin(); it != _aux_sys._elem_vars[_tid].end(); ++it)
  {
    MooseVariable * var = it->second;
    var->prepare_aux();
  }

  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);

  unsigned int subdomain = elem->subdomain_id();
  if (subdomain != _subdomain)
  {
    _fe_problem.subdomainSetup(subdomain, _tid);
    _marker_whs[_tid].updateActiveMarkers(subdomain);

    const std::vector<Marker *> & markers = _marker_whs[_tid].active();
    for (std::vector<Marker *>::const_iterator it = markers.begin(); it != markers.end(); ++it)
      (*it)->subdomainSetup();

    //   if (_aux_sys._doing_dg) _aux_sys._dg_kernels[_tid].updateActiveDGKernels(_fe_problem.time(), _fe_problem.dt());
  }

  _fe_problem.reinitMaterials(subdomain, _tid);

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
ComputeMarkerThread::onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id)
{
  /*
  std::vector<IntegratedBC *> bcs = _aux_sys._bcs[_tid].activeIntegrated(bnd_id);
  if (bcs.size() > 0)
  {
    _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);

    unsigned int subdomain = elem->subdomain_id();
    if (subdomain != _subdomain)
      _fe_problem.subdomainSetupSide(subdomain, _tid);

    _fe_problem.reinitMaterialsFace(elem->subdomain_id(), side, _tid);
    _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

    for (std::vector<IntegratedBC *>::iterator it = bcs.begin(); it != bcs.end(); ++it)
    {
      IntegratedBC * bc = (*it);
      if (bc->shouldApply())
        bc->computeMarker();
    }
  }
  */
}

void
ComputeMarkerThread::onInternalSide(const Elem *elem, unsigned int side)
{
/*
  // Pointer to the neighbor we are currently working on.
  const Elem * neighbor = elem->neighbor(side);

  // Get the global id of the element and the neighbor
  const unsigned int elem_id = elem->id();
  const unsigned int neighbor_id = neighbor->id();

  if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) || (neighbor->level() < elem->level()))
  {
    std::vector<DGKernel *> dgks = _aux_sys._dg_kernels[_tid].active();
    if (dgks.size() > 0)
    {
      _problem.reinitNeighbor(elem, side, _tid);

      _problem.reinitMaterialsFace(elem->subdomain_id(), side, _tid);
      _problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), side, _tid);
      for (std::vector<DGKernel *>::iterator it = dgks.begin(); it != dgks.end(); ++it)
      {
        DGKernel * dg = *it;
        dg->computeMarker();
      }

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _problem.addMarkerNeighbor(_marker, _tid);
      }
    }
  }*/
}

void
ComputeMarkerThread::postElement(const Elem * /*elem*/)
{
//  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
//  _problem.addMarker(_marker, _tid);
}

void
ComputeMarkerThread::join(const ComputeMarkerThread & /*y*/)
{
}
