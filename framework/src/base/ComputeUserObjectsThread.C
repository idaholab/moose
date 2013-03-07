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
}

void
ComputeUserObjectsThread::onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id)
{
  if (_user_objects[_tid].sideUserObjects(bnd_id).size() > 0)
  {
    _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);
    _fe_problem.reinitMaterialsFace(_subdomain, side, _tid);

    for (std::vector<SideUserObject *>::const_iterator side_UserObject_it = _user_objects[_tid].sideUserObjects(bnd_id, _group).begin();
         side_UserObject_it != _user_objects[_tid].sideUserObjects(bnd_id, _group).end();
         ++side_UserObject_it)
      (*side_UserObject_it)->execute();
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
