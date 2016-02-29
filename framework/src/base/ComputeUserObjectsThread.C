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

#include "ComputeUserObjectsThread.h"
#include "Problem.h"
#include "SystemBase.h"
#include "ElementUserObject.h"
#include "SideUserObject.h"
#include "InternalSideUserObject.h"
#include "NodalUserObject.h"

#include "libmesh/numeric_vector.h"


ComputeUserObjectsThread::ComputeUserObjectsThread(FEProblem & problem,
                                                   SystemBase & sys,
                                                   const MooseObjectWarehouse<ElementUserObject> & elemental_user_objects,
                                                   const MooseObjectWarehouse<SideUserObject> & side_user_objects,
                                                   const MooseObjectWarehouse<InternalSideUserObject> & internal_side_user_objects) :
    ThreadedElementLoop<ConstElemRange>(problem, sys),
    _soln(*sys.currentSolution()),
    _elemental_user_objects(elemental_user_objects),
    _side_user_objects(side_user_objects),
    _internal_side_user_objects(internal_side_user_objects)
{
}

// Splitting Constructor
ComputeUserObjectsThread::ComputeUserObjectsThread(ComputeUserObjectsThread & x, Threads::split) :
    ThreadedElementLoop<ConstElemRange>(x._fe_problem, x._system),
    _soln(x._soln),
    _elemental_user_objects(x._elemental_user_objects),
    _side_user_objects(x._side_user_objects),
    _internal_side_user_objects(x._internal_side_user_objects)
{
}

ComputeUserObjectsThread::~ComputeUserObjectsThread()
{
}

void
ComputeUserObjectsThread::subdomainChanged()
{
  std::set<MooseVariable *> needed_moose_vars;
  _elemental_user_objects.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _side_user_objects.updateBoundaryVariableDependency(needed_moose_vars, _tid);
  _internal_side_user_objects.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);

  _elemental_user_objects.subdomainSetup(_subdomain, _tid);
  _side_user_objects.subdomainSetup(_tid);
  _internal_side_user_objects.subdomainSetup(_subdomain, _tid);

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
ComputeUserObjectsThread::onElement(const Elem * elem)
{
  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);
  _fe_problem.reinitMaterials(_subdomain, _tid);

  if (_elemental_user_objects.hasActiveBlockObjects(_subdomain, _tid))
  {
    const std::vector<MooseSharedPointer<ElementUserObject> > & objects = _elemental_user_objects.getActiveBlockObjects(_subdomain, _tid);
    for (std::vector<MooseSharedPointer<ElementUserObject> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
      (*it)->execute();
  }

  _fe_problem.swapBackMaterials(_tid);
}

void
ComputeUserObjectsThread::onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id)
{
  if (_side_user_objects.hasActiveBoundaryObjects(bnd_id, _tid))
  {
    _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);
    _fe_problem.reinitMaterialsFace(_subdomain, _tid);
    _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);
    _fe_problem.setCurrentBoundaryID(bnd_id);

    const std::vector<MooseSharedPointer<SideUserObject> > & objects = _side_user_objects.getActiveBoundaryObjects(bnd_id, _tid);
    for (std::vector<MooseSharedPointer<SideUserObject> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
      (*it)->execute();

    _fe_problem.setCurrentBoundaryID(Moose::INVALID_BOUNDARY_ID);
    _fe_problem.swapBackMaterialsFace(_tid);
  }
}

void
ComputeUserObjectsThread::onInternalSide(const Elem *elem, unsigned int side)
{
  // Pointer to the neighbor we are currently working on.
  const Elem * neighbor = elem->neighbor(side);

  // Get the global id of the element and the neighbor
  const dof_id_type
    elem_id = elem->id(),
    neighbor_id = neighbor->id();

  if (_internal_side_user_objects.hasActiveBlockObjects(_subdomain, _tid))
  {
    if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) || (neighbor->level() < elem->level()))
    {
      _fe_problem.prepareFace(elem, _tid);
      _fe_problem.reinitNeighbor(elem, side, _tid);
      _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
      _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

      const std::vector<MooseSharedPointer<InternalSideUserObject> > & objects = _internal_side_user_objects.getActiveBlockObjects(_subdomain, _tid);
      for (std::vector<MooseSharedPointer<InternalSideUserObject> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
      {
        if ( !(*it)->blockRestricted())
          (*it)->execute();

        else if ( (*it)->hasBlocks(neighbor->subdomain_id()) )
          (*it)->execute();
      }
      _fe_problem.swapBackMaterialsFace(_tid);
      _fe_problem.swapBackMaterialsNeighbor(_tid);
    }
  }
}

void
ComputeUserObjectsThread::post()
{
  _fe_problem.clearActiveElementalMooseVariables(_tid);
}

void
ComputeUserObjectsThread::join(const ComputeUserObjectsThread & /*y*/)
{
}
