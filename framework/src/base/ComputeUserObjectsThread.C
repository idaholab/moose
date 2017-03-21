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
#include "ShapeElementUserObject.h"
#include "SideUserObject.h"
#include "ShapeSideUserObject.h"
#include "InternalSideUserObject.h"
#include "NodalUserObject.h"
#include "SwapBackSentinel.h"
#include "FEProblem.h"

#include "libmesh/numeric_vector.h"

ComputeUserObjectsThread::ComputeUserObjectsThread(
    FEProblemBase & problem,
    SystemBase & sys,
    const MooseObjectWarehouse<ElementUserObject> & elemental_user_objects,
    const MooseObjectWarehouse<SideUserObject> & side_user_objects,
    const MooseObjectWarehouse<InternalSideUserObject> & internal_side_user_objects)
  : ThreadedElementLoop<ConstElemRange>(problem),
    _soln(*sys.currentSolution()),
    _elemental_user_objects(elemental_user_objects),
    _side_user_objects(side_user_objects),
    _internal_side_user_objects(internal_side_user_objects)
{
}

// Splitting Constructor
ComputeUserObjectsThread::ComputeUserObjectsThread(ComputeUserObjectsThread & x, Threads::split)
  : ThreadedElementLoop<ConstElemRange>(x._fe_problem),
    _soln(x._soln),
    _elemental_user_objects(x._elemental_user_objects),
    _side_user_objects(x._side_user_objects),
    _internal_side_user_objects(x._internal_side_user_objects)
{
}

ComputeUserObjectsThread::~ComputeUserObjectsThread() {}

void
ComputeUserObjectsThread::subdomainChanged()
{
  std::set<MooseVariable *> needed_moose_vars;
  _elemental_user_objects.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _side_user_objects.updateBoundaryVariableDependency(needed_moose_vars, _tid);
  _internal_side_user_objects.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);

  std::set<unsigned int> needed_mat_props;
  _elemental_user_objects.updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);
  _side_user_objects.updateBoundaryMatPropDependency(needed_mat_props, _tid);
  _internal_side_user_objects.updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);

  _elemental_user_objects.subdomainSetup(_subdomain, _tid);
  _side_user_objects.subdomainSetup(_tid);
  _internal_side_user_objects.subdomainSetup(_subdomain, _tid);

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
ComputeUserObjectsThread::onElement(const Elem * elem)
{
  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);

  // Set up Sentinel class so that, even if reinitMaterials() throws, we
  // still remember to swap back during stack unwinding.
  SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterials, _tid);
  _fe_problem.reinitMaterials(_subdomain, _tid);

  if (_elemental_user_objects.hasActiveBlockObjects(_subdomain, _tid))
  {
    const auto & objects = _elemental_user_objects.getActiveBlockObjects(_subdomain, _tid);
    for (const auto & uo : objects)
      uo->execute();
  }

  // UserObject Jacobians
  if (_fe_problem.currentlyComputingJacobian() &&
      _elemental_user_objects.hasActiveBlockObjects(_subdomain, _tid))
  {
    // Prepare shape functions for ShapeElementUserObjects
    std::vector<MooseVariable *> jacobian_moose_vars =
        _fe_problem.getUserObjectJacobianVariables(_tid);
    for (auto & jvar : jacobian_moose_vars)
    {
      unsigned int jvar_id = jvar->number();
      std::vector<dof_id_type> & dof_indices = jvar->dofIndices();

      _fe_problem.prepareShapes(jvar_id, _tid);

      const auto & e_objects = _elemental_user_objects.getActiveBlockObjects(_subdomain, _tid);
      for (const auto & uo : e_objects)
      {
        auto shape_element_uo = std::dynamic_pointer_cast<ShapeElementUserObject>(uo);
        if (shape_element_uo)
          shape_element_uo->executeJacobianWrapper(jvar_id, dof_indices);
      }
    }
  }
}

void
ComputeUserObjectsThread::onBoundary(const Elem * elem, unsigned int side, BoundaryID bnd_id)
{
  if (!_side_user_objects.hasActiveBoundaryObjects(bnd_id, _tid))
    return;

  _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);

  // Set up Sentinel class so that, even if reinitMaterialsFace() throws, we
  // still remember to swap back during stack unwinding.
  SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);
  _fe_problem.reinitMaterialsFace(_subdomain, _tid);
  _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

  _fe_problem.setCurrentBoundaryID(bnd_id);

  const auto & objects = _side_user_objects.getActiveBoundaryObjects(bnd_id, _tid);
  for (const auto & uo : objects)
    uo->execute();

  // UserObject Jacobians
  if (_fe_problem.currentlyComputingJacobian())
  {
    // Prepare shape functions for ShapeSideUserObjects
    std::vector<MooseVariable *> jacobian_moose_vars =
        _fe_problem.getUserObjectJacobianVariables(_tid);
    for (auto & jvar : jacobian_moose_vars)
    {
      unsigned int jvar_id = jvar->number();
      std::vector<dof_id_type> & dof_indices = jvar->dofIndices();

      _fe_problem.prepareFaceShapes(jvar_id, _tid);

      for (const auto & uo : objects)
      {
        auto shape_side_uo = std::dynamic_pointer_cast<ShapeSideUserObject>(uo);
        if (shape_side_uo)
          shape_side_uo->executeJacobianWrapper(jvar_id, dof_indices);
      }
    }
  }

  _fe_problem.setCurrentBoundaryID(Moose::INVALID_BOUNDARY_ID);
}

void
ComputeUserObjectsThread::onInternalSide(const Elem * elem, unsigned int side)
{
  // Pointer to the neighbor we are currently working on.
  const Elem * neighbor = elem->neighbor(side);

  // Get the global id of the element and the neighbor
  const dof_id_type elem_id = elem->id(), neighbor_id = neighbor->id();

  if (!_internal_side_user_objects.hasActiveBlockObjects(_subdomain, _tid))
    return;
  if (!((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) ||
        (neighbor->level() < elem->level())))
    return;

  _fe_problem.prepareFace(elem, _tid);
  _fe_problem.reinitNeighbor(elem, side, _tid);

  // Set up Sentinels so that, even if one of the reinitMaterialsXXX() calls throws, we
  // still remember to swap back during stack unwinding.
  SwapBackSentinel face_sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);
  _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);

  SwapBackSentinel neighbor_sentinel(_fe_problem, &FEProblem::swapBackMaterialsNeighbor, _tid);
  _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

  const auto & objects = _internal_side_user_objects.getActiveBlockObjects(_subdomain, _tid);
  for (const auto & uo : objects)
  {
    if (!uo->blockRestricted())
      uo->execute();
    else if (uo->hasBlocks(neighbor->subdomain_id()))
      uo->execute();
  }
}

void
ComputeUserObjectsThread::post()
{
  _fe_problem.clearActiveElementalMooseVariables(_tid);
  _fe_problem.clearActiveMaterialProperties(_tid);
}

void
ComputeUserObjectsThread::join(const ComputeUserObjectsThread & /*y*/)
{
}
