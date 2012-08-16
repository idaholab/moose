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

ComputeUserObjectsThread::ComputeUserObjectsThread(FEProblem & problem, SystemBase & sys, const NumericVector<Number>& in_soln, std::vector<UserObjectWarehouse> & user_objects, UserObjectWarehouse::GROUP group) :
    ThreadedElementLoop<ConstElemRange>(problem, sys),
    _soln(in_soln),
    _user_objects(user_objects),
    _group(group)
{}

// Splitting Constructor
ComputeUserObjectsThread::ComputeUserObjectsThread(ComputeUserObjectsThread & x, Threads::split) :
    ThreadedElementLoop<ConstElemRange>(x._fe_problem, x._system),
    _soln(x._soln),
    _user_objects(x._user_objects)
{}

void
ComputeUserObjectsThread::onElement(const Elem * elem)
{
  unsigned int subdomain = elem->subdomain_id();

  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);
  _fe_problem.reinitMaterials(subdomain, _tid);

  //Global UserObjects
  for (std::vector<ElementUserObject *>::const_iterator UserObject_it = _user_objects[_tid].elementUserObjects(Moose::ANY_BLOCK_ID, _group).begin();
       UserObject_it != _user_objects[_tid].elementUserObjects(Moose::ANY_BLOCK_ID, _group).end();
       ++UserObject_it)
    (*UserObject_it)->execute();

  for (std::vector<ElementUserObject *>::const_iterator UserObject_it = _user_objects[_tid].elementUserObjects(subdomain, _group).begin();
       UserObject_it != _user_objects[_tid].elementUserObjects(subdomain, _group).end();
       ++UserObject_it)
    (*UserObject_it)->execute();
}

void
ComputeUserObjectsThread::onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id)
{
  if (_user_objects[_tid].sideUserObjects(bnd_id).size() > 0)
  {
    _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);
    _fe_problem.reinitMaterialsFace(elem->subdomain_id(), side, _tid);

    for (std::vector<SideUserObject *>::const_iterator side_UserObject_it = _user_objects[_tid].sideUserObjects(bnd_id, _group).begin();
         side_UserObject_it != _user_objects[_tid].sideUserObjects(bnd_id, _group).end();
         ++side_UserObject_it)
      (*side_UserObject_it)->execute();
  }
}

void
ComputeUserObjectsThread::join(const ComputeUserObjectsThread & /*y*/)
{
}
