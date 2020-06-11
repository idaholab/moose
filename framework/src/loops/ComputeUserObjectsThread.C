//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeUserObjectsThread.h"
#include "Problem.h"
#include "SystemBase.h"
#include "ElementUserObject.h"
#include "ShapeElementUserObject.h"
#include "SideUserObject.h"
#include "InterfaceUserObject.h"
#include "ShapeSideUserObject.h"
#include "InternalSideUserObject.h"
#include "NodalUserObject.h"
#include "SwapBackSentinel.h"
#include "FEProblem.h"

#include "libmesh/numeric_vector.h"

ComputeUserObjectsThread::ComputeUserObjectsThread(FEProblemBase & problem,
                                                   SystemBase & sys,
                                                   const TheWarehouse::Query & query)
  : ThreadedElementLoop<ConstElemRange>(problem),
    _soln(*sys.currentSolution()),
    _query(query),
    _query_subdomain(_query),
    _query_boundary(_query)
{
}

// Splitting Constructor
ComputeUserObjectsThread::ComputeUserObjectsThread(ComputeUserObjectsThread & x, Threads::split)
  : ThreadedElementLoop<ConstElemRange>(x._fe_problem),
    _soln(x._soln),
    _query(x._query),
    _query_subdomain(x._query_subdomain),
    _query_boundary(x._query_boundary)
{
}

ComputeUserObjectsThread::~ComputeUserObjectsThread() {}

void
ComputeUserObjectsThread::subdomainChanged()
{
  // for the current thread get block objects for the current subdomain and *all* side objects
  std::vector<UserObject *> objs;
  querySubdomain(Interfaces::ElementUserObject | Interfaces::InternalSideUserObject |
                     Interfaces::InterfaceUserObject,
                 objs);

  std::vector<UserObject *> side_objs;
  _query.clone()
      .condition<AttribThread>(_tid)
      .condition<AttribInterfaces>(Interfaces::SideUserObject)
      .queryInto(side_objs);

  objs.insert(objs.begin(), side_objs.begin(), side_objs.end());

  // collect dependenciesand run subdomain setup
  _fe_problem.subdomainSetup(_subdomain, _tid);

  std::set<MooseVariableFEBase *> needed_moose_vars;
  std::set<unsigned int> needed_mat_props;
  for (const auto obj : objs)
  {
    auto v_obj = dynamic_cast<MooseVariableDependencyInterface *>(obj);
    if (!v_obj)
      mooseError("robert wrote broken code");
    const auto & v_deps = v_obj->getMooseVariableDependencies();
    needed_moose_vars.insert(v_deps.begin(), v_deps.end());

    auto m_obj = dynamic_cast<MaterialPropertyInterface *>(obj);
    if (!m_obj)
      mooseError("robert wrote broken code again");
    auto & m_deps = m_obj->getMatPropDependencies();
    needed_mat_props.insert(m_deps.begin(), m_deps.end());

    obj->subdomainSetup();
  }

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);

  querySubdomain(Interfaces::InternalSideUserObject, _internal_side_objs);
  querySubdomain(Interfaces::InterfaceUserObject, _interface_user_objects);
  querySubdomain(Interfaces::ElementUserObject, _element_objs);
  querySubdomain(Interfaces::ShapeElementUserObject, _shape_element_objs);
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

  for (const auto & uo : _element_objs)
    uo->execute();

  // UserObject Jacobians
  if (_fe_problem.currentlyComputingJacobian() && _shape_element_objs.size() > 0)
  {
    // Prepare shape functions for ShapeElementUserObjects
    const auto & jacobian_moose_vars = _fe_problem.getUserObjectJacobianVariables(_tid);
    for (const auto & jvar : jacobian_moose_vars)
    {
      unsigned int jvar_id = jvar->number();
      auto && dof_indices = jvar->dofIndices();

      _fe_problem.prepareShapes(jvar_id, _tid);
      for (const auto uo : _shape_element_objs)
        uo->executeJacobianWrapper(jvar_id, dof_indices);
    }
  }
}

void
ComputeUserObjectsThread::onBoundary(const Elem * elem,
                                     unsigned int side,
                                     BoundaryID bnd_id,
                                     const Elem * lower_d_elem /*=nullptr*/)
{
  std::vector<UserObject *> userobjs;
  queryBoundary(Interfaces::SideUserObject, bnd_id, userobjs);
  if (userobjs.size() == 0)
    return;

  _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);

  // Reinitialize lower-dimensional variables for use in boundary Materials
  if (lower_d_elem)
    _fe_problem.reinitLowerDElem(lower_d_elem, _tid);

  // Set up Sentinel class so that, even if reinitMaterialsFace() throws, we
  // still remember to swap back during stack unwinding.
  SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);
  _fe_problem.reinitMaterialsFace(_subdomain, _tid);
  _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

  for (const auto & uo : userobjs)
    uo->execute();

  // UserObject Jacobians
  std::vector<ShapeSideUserObject *> shapers;
  queryBoundary(Interfaces::ShapeSideUserObject, bnd_id, shapers);
  if (_fe_problem.currentlyComputingJacobian() && shapers.size() > 0)
  {
    // Prepare shape functions for ShapeSideUserObjects
    const auto & jacobian_moose_vars = _fe_problem.getUserObjectJacobianVariables(_tid);
    for (const auto & jvar : jacobian_moose_vars)
    {
      unsigned int jvar_id = jvar->number();
      auto && dof_indices = jvar->dofIndices();

      _fe_problem.prepareFaceShapes(jvar_id, _tid);

      for (const auto & uo : shapers)
        uo->executeJacobianWrapper(jvar_id, dof_indices);
    }
  }
}

void
ComputeUserObjectsThread::onInternalSide(const Elem * elem, unsigned int side)
{
  // Pointer to the neighbor we are currently working on.
  const Elem * neighbor = elem->neighbor_ptr(side);

  // Get the global id of the element and the neighbor
  const dof_id_type elem_id = elem->id(), neighbor_id = neighbor->id();

  if (_internal_side_objs.size() == 0)
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

  for (const auto & uo : _internal_side_objs)
    if (!uo->blockRestricted() || uo->hasBlocks(neighbor->subdomain_id()))
      uo->execute();
}

void
ComputeUserObjectsThread::onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id)
{
  // Pointer to the neighbor we are currently working on.
  const Elem * neighbor = elem->neighbor_ptr(side);

  std::vector<UserObject *> userobjs;
  queryBoundary(Interfaces::InterfaceUserObject, bnd_id, userobjs);
  if (_interface_user_objects.size() == 0)
    return;
  if (!(neighbor->active()))
    return;

  _fe_problem.prepareFace(elem, _tid);
  _fe_problem.reinitNeighbor(elem, side, _tid);

  // Set up Sentinels so that, even if one of the reinitMaterialsXXX() calls throws, we
  // still remember to swap back during stack unwinding.

  SwapBackSentinel face_sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);
  _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
  _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

  SwapBackSentinel neighbor_sentinel(_fe_problem, &FEProblem::swapBackMaterialsNeighbor, _tid);
  _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

  // Has to happen after face and neighbor properties have been computed. Note that we don't use
  // a sentinel here because FEProblem::swapBackMaterialsFace is going to handle face materials,
  // boundary materials, and interface materials (e.g. it queries the boundary material data
  // with the current element and side
  _fe_problem.reinitMaterialsInterface(bnd_id, _tid);

  for (const auto & uo : userobjs)
    uo->execute();
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
