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


ComputeUserObjectsThread::ComputeUserObjectsThread(FEProblem & problem, SystemBase & sys, const NumericVector<Number>& in_soln, std::vector<UserObjectWarehouse> & user_objects, UserObjectWarehouse::GROUP group) :
    ThreadedElementLoop<ConstElemRange>(problem, sys),
    _soln(in_soln),
    _user_objects(user_objects),
    _group(group)
{
}

// Splitting Constructor
ComputeUserObjectsThread::ComputeUserObjectsThread(ComputeUserObjectsThread & x, Threads::split) :
    ThreadedElementLoop<ConstElemRange>(x._fe_problem, x._system),
    _soln(x._soln),
    _user_objects(x._user_objects),
    _group(x._group)
{
}

ComputeUserObjectsThread::~ComputeUserObjectsThread()
{
}

void
ComputeUserObjectsThread::subdomainChanged()
{
  std::set<MooseVariable *> needed_moose_vars;

  for (std::vector<ElementUserObject *>::const_iterator UserObject_it = _user_objects[_tid].elementUserObjects(Moose::ANY_BLOCK_ID, _group).begin();
       UserObject_it != _user_objects[_tid].elementUserObjects(Moose::ANY_BLOCK_ID, _group).end();
       ++UserObject_it)
  {
    const std::set<MooseVariable *> & mv_deps = (*UserObject_it)->getMooseVariableDependencies();
    needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
  }

  for (std::vector<ElementUserObject *>::const_iterator UserObject_it = _user_objects[_tid].elementUserObjects(_subdomain, _group).begin();
       UserObject_it != _user_objects[_tid].elementUserObjects(_subdomain, _group).end();
       ++UserObject_it)
  {
    const std::set<MooseVariable *> & mv_deps = (*UserObject_it)->getMooseVariableDependencies();
    needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
  }

  // Boundary UserObject Dependencies
  const std::set<unsigned int> & subdomain_boundary_ids = _mesh.getSubdomainBoundaryIds(_subdomain);
  for(std::set<unsigned int>::const_iterator id_it = subdomain_boundary_ids.begin();
      id_it != subdomain_boundary_ids.end();
      ++id_it)
  {
    for (std::vector<SideUserObject *>::const_iterator side_UserObject_it = _user_objects[_tid].sideUserObjects(*id_it, _group).begin();
         side_UserObject_it != _user_objects[_tid].sideUserObjects(*id_it, _group).end();
         ++side_UserObject_it)
    {
      const std::set<MooseVariable *> & mv_deps = (*side_UserObject_it)->getMooseVariableDependencies();
      needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }
  }

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
ComputeUserObjectsThread::onElement(const Elem * elem)
{
  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);
  _fe_problem.reinitMaterials(_subdomain, _tid);

  //Global UserObjects
  for (std::vector<ElementUserObject *>::const_iterator UserObject_it = _user_objects[_tid].elementUserObjects(Moose::ANY_BLOCK_ID, _group).begin();
       UserObject_it != _user_objects[_tid].elementUserObjects(Moose::ANY_BLOCK_ID, _group).end();
       ++UserObject_it)
    (*UserObject_it)->execute();

  for (std::vector<ElementUserObject *>::const_iterator UserObject_it = _user_objects[_tid].elementUserObjects(_subdomain, _group).begin();
       UserObject_it != _user_objects[_tid].elementUserObjects(_subdomain, _group).end();
       ++UserObject_it)
    (*UserObject_it)->execute();

  _fe_problem.swapBackMaterials(_tid);
}

void
ComputeUserObjectsThread::onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id)
{
  if (_user_objects[_tid].sideUserObjects(bnd_id).size() > 0)
  {
    _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);
    _fe_problem.reinitMaterialsFace(_subdomain, _tid);

    for (std::vector<SideUserObject *>::const_iterator side_UserObject_it = _user_objects[_tid].sideUserObjects(bnd_id, _group).begin();
         side_UserObject_it != _user_objects[_tid].sideUserObjects(bnd_id, _group).end();
         ++side_UserObject_it)
    {
      _fe_problem.setCurrentBoundaryID(bnd_id);
      (*side_UserObject_it)->execute();
    }
    _fe_problem.setCurrentBoundaryID(Moose::INVALID_BOUNDARY_ID);
    _fe_problem.swapBackMaterialsFace(_tid);
  }
}

void
ComputeUserObjectsThread::onInternalSide(const Elem *elem, unsigned int side)
{
  const std::vector<InternalSideUserObject *> & isuo = _user_objects[_tid].internalSideUserObjects();
  if (isuo.size() > 0)
  {
    // Pointer to the neighbor we are currently working on.
    const Elem * neighbor = elem->neighbor(side);

    // Get the global id of the element and the neighbor
    const unsigned int elem_id = elem->id();
    const unsigned int neighbor_id = neighbor->id();

    if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) || (neighbor->level() < elem->level()))
    {
      _fe_problem.prepareFace(elem, _tid);
      _fe_problem.reinitNeighbor(elem, side, _tid);
      _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
      _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

      for (std::vector<InternalSideUserObject *>::const_iterator it = isuo.begin(); it != isuo.end(); ++it)
      {
        InternalSideUserObject * uo = *it;
        uo->execute();
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
