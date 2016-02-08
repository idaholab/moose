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

  _side_user_objects.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _side_user_objects.updateBoundaryVariableDependency(needed_moose_vars, _tid);
  _side_user_objects.updateVariableDependency(needed_moose_vars, _tid);

  _internal_side_user_objects.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  //_internal_side_user_objects.updateBoundaryVariableDependency(needed_moose_vars);
  //_internal_side_user_objects.updateVariableDependency(needed_moose_vars);


  /*
  // ElementUserObject dependencies
  {
    // Get the vectors of element user object pointers
    const std::vector<ElementUserObject *> global = _user_objects[_tid].elementUserObjects(Moose::ANY_BLOCK_ID, _group);
    const std::vector<ElementUserObject *> block = _user_objects[_tid].elementUserObjects(_subdomain, _group);

    // Global ElementUserObjects
    for (std::vector<ElementUserObject *>::const_iterator it = global.begin(); it != global.end(); ++it)
    {
      const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
      needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }

    // Block Restricted ElementUserObjects
    for (std::vector<ElementUserObject *>::const_iterator it = block.begin(); it != block.end(); ++it)
    {
      const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
      needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }
  }

  // InternalSideUserObject dependencies
  {
    // Get the vectors of element user object pointers
    const std::vector<InternalSideUserObject *> global = _user_objects[_tid].internalSideUserObjects(Moose::ANY_BLOCK_ID, _group);
    const std::vector<InternalSideUserObject *> block = _user_objects[_tid].internalSideUserObjects(_subdomain, _group);

    // Global InternalSideUserObjects
    for (std::vector<InternalSideUserObject *>::const_iterator it = global.begin(); it != global.end(); ++it)
    {
      const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
      needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }

    // Block Restricted InternalSideUserObjects
    for (std::vector<InternalSideUserObject *>::const_iterator it = block.begin(); it != block.end(); ++it)
    {
      const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
      needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }
  }

  // NodalUserObject dependencies (block restricted)
  {
    // Get the vectors of element user object pointers
    const std::vector<NodalUserObject *> block = _user_objects[_tid].blockNodalUserObjects(_subdomain, _group);

    // Block Restricted NodalUserObjects
    for (std::vector<NodalUserObject *>::const_iterator it = block.begin(); it != block.end(); ++it)
    {
      const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
      needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }
  }

  // Boundary UserObject Dependencies (SideUserObjects and NodalUserObjects)
  const std::set<unsigned int> & bnd_ids = _mesh.getSubdomainBoundaryIds(_subdomain);
  for (std::set<unsigned int>::const_iterator id_it = bnd_ids.begin(); id_it != bnd_ids.end(); ++id_it)
  {
    // SideUserObjects
    {
      // Get the vectors of user object pointers
      const std::vector<SideUserObject *> global = _user_objects[_tid].sideUserObjects(Moose::ANY_BOUNDARY_ID, _group);
      const std::vector<SideUserObject *> boundary = _user_objects[_tid].sideUserObjects(*id_it, _group);

      // Global SideUserObjects
      for (std::vector<SideUserObject *>::const_iterator it = global.begin(); it != global.end(); ++it)
      {
        const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
        needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
      }

      // Boundary Restricted InternalSideUserObjects
      for (std::vector<SideUserObject *>::const_iterator it = boundary.begin(); it != boundary.end(); ++it)
      {
        const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
        needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
      }
    }

    // NodalUserObjects
    {
      // Get the vectors of user object pointers
      const std::vector<NodalUserObject *> global = _user_objects[_tid].nodalUserObjects(Moose::ANY_BOUNDARY_ID, _group);
      const std::vector<NodalUserObject *> boundary = _user_objects[_tid].nodalUserObjects(*id_it, _group);

      // Global SideUserObjects
      for (std::vector<NodalUserObject *>::const_iterator it = global.begin(); it != global.end(); ++it)
      {
        const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
        needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
      }
mk
      // Boundary Restricted InternalSideUserObjects
      for (std::vector<NodalUserObject *>::const_iterator it = boundary.begin(); it != boundary.end(); ++it)
      {
        const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
        needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
      }
    }
  }
  */
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


  /*
  if (elem->on_boundary())
  {
    std::vector<BoundaryID> ids;
    for (unsigned int side = 0; side < elem->n_sides(); ++side)
      _fe_problem.mesh().getMesh().get_boundary_info().boundary_ids(elem, side, ids);

    std::set<BoundaryID> bnd_ids(ids.begin(), ids.end());

    for (std::set<BoundaryID>::const_iterator bnd_it = bnd_ids.begin(); bnd_it != bnd_ids.end(); ++bnd_it)
    {
      if (_elemental_user_objects.hasActiveBoundaryObjects(*bnd_it, _tid))
      {
        const std::vector<MooseSharedPointer<ElementUserObject> > & objects = _elemental_user_objects.getActiveBoundaryObjects(*bnd_it, _tid);
        for (std::vector<MooseSharedPointer<ElementUserObject> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
          (*it)->execute();
      }
    }
  }
  */


  _fe_problem.swapBackMaterials(_tid);

  /*
  //Global UserObjects
  for (std::vector<ElementUserObject *>::const_iterator UserObject_it = _user_objects[_tid].elementUserObjects(Moose::ANY_BLOCK_ID, _group).begin();
       UserObject_it != _user_objects[_tid].elementUserObjects(Moose::ANY_BLOCK_ID, _group).end();
       ++UserObject_it)
    (*UserObject_it)->execute();

  for (std::vector<ElementUserObject *>::const_iterator UserObject_it = _user_objects[_tid].elementUserObjects(_subdomain, _group).begin();
       UserObject_it != _user_objects[_tid].elementUserObjects(_subdomain, _group).end();
       ++UserObject_it)
    (*UserObject_it)->execute();
  */


}

void
ComputeUserObjectsThread::onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id)
{
  bool has_side = _side_user_objects.hasActiveBoundaryObjects(bnd_id, _tid);
  bool has_elemental = _elemental_user_objects.hasActiveBoundaryObjects(bnd_id, _tid);

  if (has_side || has_elemental)
  {
    _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);
    _fe_problem.reinitMaterialsFace(_subdomain, _tid);
    _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);
    _fe_problem.setCurrentBoundaryID(bnd_id);

    /*
    if (has_elemental)
    {
      const std::vector<MooseSharedPointer<ElementUserObject> > & objects = _elemental_user_objects.getActiveBoundaryObjects(bnd_id, _tid);
      for (std::vector<MooseSharedPointer<ElementUserObject> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
        (*it)->execute();
    }
    */

    if (has_side)
    {
      const std::vector<MooseSharedPointer<SideUserObject> > & objects = _side_user_objects.getActiveBoundaryObjects(bnd_id, _tid);
      for (std::vector<MooseSharedPointer<SideUserObject> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
        (*it)->execute();
    }

    _fe_problem.setCurrentBoundaryID(Moose::INVALID_BOUNDARY_ID);
    _fe_problem.swapBackMaterialsFace(_tid);
  }


  /*
  if (_user_objects[_tid].sideUserObjects(bnd_id).size() > 0)
  {
    _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);
    _fe_problem.reinitMaterialsFace(_subdomain, _tid);
    _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

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
  */
}

void
ComputeUserObjectsThread::onInternalSide(const Elem *elem, unsigned int side)
{
  //
  //const std::vector<MooseSharedPointer<InternalSideUserObject> > & global = _internal_side_user_objects.getActiveBlockObjects(Moose::ANY_BLOCK_ID, _tid);
  //const std::vector<MooseSharedPointer<InternalSideUserObject> > & blocks = _internal_side_user_objects.getActiveBlockObjects(_subdomain, _tid);

  bool has_global = _internal_side_user_objects.hasActiveBlockObjects(Moose::ANY_BLOCK_ID, _tid);
  bool has_blocks = _internal_side_user_objects.hasActiveBlockObjects(_subdomain, _tid);

  // Pointer to the neighbor we are currently working on.
  const Elem * neighbor = elem->neighbor(side);

  // Get the global id of the element and the neighbor
  const dof_id_type
    elem_id = elem->id(),
    neighbor_id = neighbor->id();

  if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) || (neighbor->level() < elem->level()))
  {

    if (has_global || has_blocks)
    {
      _fe_problem.prepareFace(elem, _tid);
      _fe_problem.reinitNeighbor(elem, side, _tid);
      _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
      _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

      // Global
      if (has_global)
      {
        //   const std::vector<MooseSharedPointer<InternalSideUserObject> > & objects = _internal_side_user_objects.getActiveBlockObjects(Moose::ANY_BLOCK_ID, _tid);
        //for (std::vector<MooseSharedPointer<InternalSideUserObject> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
        //  (*it)->execute();
      }

      if (has_blocks)
      {
        const std::vector<MooseSharedPointer<InternalSideUserObject> > & objects = _internal_side_user_objects.getActiveBlockObjects(_subdomain, _tid);
        for (std::vector<MooseSharedPointer<InternalSideUserObject> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
        {
          if ( !(*it)->blockRestricted())
            (*it)->execute();

          else if ( (*it)->hasBlocks(neighbor->subdomain_id()) )
            (*it)->execute();
        }
      }

      _fe_problem.swapBackMaterialsFace(_tid);
      _fe_problem.swapBackMaterialsNeighbor(_tid);

    }
  }



  /*
  // Get vectors of object pointers
  const std::vector<InternalSideUserObject *> & block_uo = _user_objects[_tid].internalSideUserObjects(_subdomain, _group);
  const std::vector<InternalSideUserObject *> & global_uo = _user_objects[_tid].internalSideUserObjects(Moose::ANY_BLOCK_ID, _group);

  // Pointer to the neighbor we are currently working on.
  const Elem * neighbor = elem->neighbor(side);

  // Get the global id of the element and the neighbor
  const dof_id_type
    elem_id = elem->id(),
    neighbor_id = neighbor->id();

  // Only perform execute if there are objects
  if (global_uo.size() > 0 || block_uo.size() > 0)
  {
    if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) || (neighbor->level() < elem->level()))
    {
      // Why are you always preparing...just go
      _fe_problem.prepareFace(elem, _tid);
      _fe_problem.reinitNeighbor(elem, side, _tid);
      _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
      _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

      // Execute Global InternalSideUserObjects
      for (std::vector<InternalSideUserObject *>::const_iterator it = global_uo.begin(); it != global_uo.end(); ++it)
        (*it)->execute();

      // Loop through the block restricted objects
      for (std::vector<InternalSideUserObject *>::const_iterator it = block_uo.begin(); it != block_uo.end(); ++it)
        {
          // If the neighbor subdomain is a member of the blocks to which the current object is restricted the run execute
          if ( (*it)->hasBlocks(neighbor->subdomain_id()) )
            (*it)->execute();
        }

      _fe_problem.swapBackMaterialsFace(_tid);
      _fe_problem.swapBackMaterialsNeighbor(_tid);
    }
  }
  */

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
