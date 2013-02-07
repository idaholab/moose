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

// libmesh includes
#include "libmesh/threads.h"

ComputeIndicatorThread::ComputeIndicatorThread(FEProblem & fe_problem,
                                               AuxiliarySystem & sys,
                                               std::vector<IndicatorWarehouse> & indicator_whs,
                                               bool finalize) :
    ThreadedElementLoop<ConstElemRange>(fe_problem, sys),
    _fe_problem(fe_problem),
    _aux_sys(sys),
    _indicator_whs(indicator_whs),
    _finalize(finalize)
{
}

// Splitting Constructor
ComputeIndicatorThread::ComputeIndicatorThread(ComputeIndicatorThread & x, Threads::split split) :
    ThreadedElementLoop<ConstElemRange>(x, split),
    _fe_problem(x._fe_problem),
    _aux_sys(x._aux_sys),
    _indicator_whs(x._indicator_whs),
    _finalize(x._finalize)
{
}

ComputeIndicatorThread::~ComputeIndicatorThread()
{
}

void
ComputeIndicatorThread::subdomainChanged()
{
  _fe_problem.subdomainSetup(_subdomain, _tid);
  _indicator_whs[_tid].updateActiveIndicators(_subdomain);

  const std::vector<Indicator *> & indicators = _indicator_whs[_tid].active();
  for (std::vector<Indicator *>::const_iterator it = indicators.begin(); it != indicators.end(); ++it)
    (*it)->subdomainSetup();

  std::set<MooseVariable *> needed_moose_vars;

  for (std::vector<Indicator *>::const_iterator it = indicators.begin(); it != indicators.end(); ++it)
  {
    const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
    needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
  }

  const std::vector<Indicator *> & internal_side_indicators = _indicator_whs[_tid].activeInternalSideIndicators();
  if (internal_side_indicators.size() > 0)
  {
    for (std::vector<Indicator *>::const_iterator it = internal_side_indicators.begin(); it != internal_side_indicators.end(); ++it)
    {
      const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
      needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }
  }

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
ComputeIndicatorThread::onElement(const Elem *elem)
{
  for (std::map<std::string, MooseVariable *>::iterator it = _aux_sys._elem_vars[_tid].begin(); it != _aux_sys._elem_vars[_tid].end(); ++it)
  {
    MooseVariable * var = it->second;
    var->prepare_aux();
  }

  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);

  _fe_problem.reinitMaterials(_subdomain, _tid);

  const std::vector<Indicator *> & indicators = _indicator_whs[_tid].active();

  if(!_finalize)
    for (std::vector<Indicator *>::const_iterator it = indicators.begin(); it != indicators.end(); ++it)
      (*it)->computeIndicator();
  else
  {
    for (std::vector<Indicator *>::const_iterator it = indicators.begin(); it != indicators.end(); ++it)
      (*it)->finalize();

    // Now finalize the side integral side_indicators as well
    {
      const std::vector<Indicator *> & side_indicators = _indicator_whs[_tid].activeInternalSideIndicators();
      for (std::vector<Indicator *>::const_iterator it = side_indicators.begin(); it != side_indicators.end(); ++it)
        (*it)->finalize();
    }
  }

  if(!_finalize) // During finalize the Indicators should be setting values in the vectors manually
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (std::map<std::string, MooseVariable *>::iterator it = _aux_sys._elem_vars[_tid].begin(); it != _aux_sys._elem_vars[_tid].end(); ++it)
    {
      MooseVariable * var = it->second;
      var->add(_aux_sys.solution());
    }
  }
}

void
ComputeIndicatorThread::onBoundary(const Elem * /*elem*/, unsigned int /*side*/, BoundaryID /*bnd_id*/)
{
}

void
ComputeIndicatorThread::onInternalSide(const Elem *elem, unsigned int side)
{
  if(_finalize) // If finalizing we only do something on the elements
    return;

  // Pointer to the neighbor we are currently working on.
  const Elem * neighbor = elem->neighbor(side);

  // Get the global id of the element and the neighbor
  const unsigned int elem_id = elem->id();
  const unsigned int neighbor_id = neighbor->id();

  if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) || (neighbor->level() < elem->level()))
  {
    for (std::map<std::string, MooseVariable *>::iterator it = _aux_sys._elem_vars[_tid].begin(); it != _aux_sys._elem_vars[_tid].end(); ++it)
    {
      MooseVariable * var = it->second;
      var->prepare_aux();
    }

    const std::vector<Indicator *> & indicators = _indicator_whs[_tid].activeInternalSideIndicators();
    if (indicators.size() > 0)
    {
      _fe_problem.reinitNeighbor(elem, side, _tid);

      _fe_problem.reinitMaterialsFace(elem->subdomain_id(), side, _tid);
      _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), side, _tid);

      for (std::vector<Indicator *>::const_iterator it = indicators.begin(); it != indicators.end(); ++it)
        (*it)->computeIndicator();
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
}

void
ComputeIndicatorThread::join(const ComputeIndicatorThread & /*y*/)
{
}
