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
#include "MaterialBase.h"
#include "DomainUserObject.h"
#include "AuxiliarySystem.h"
#include "MooseTypes.h"

#include "libmesh/numeric_vector.h"

ComputeUserObjectsThread::ComputeUserObjectsThread(FEProblemBase & problem,
                                                   const TheWarehouse::Query & query)
  : ThreadedElementLoop<ConstElemRange>(problem),
    _query(query),
    _query_subdomain(_query),
    _query_boundary(_query),
    _aux_sys(problem.getAuxiliarySystem())
{
}

// Splitting Constructor
ComputeUserObjectsThread::ComputeUserObjectsThread(ComputeUserObjectsThread & x, Threads::split)
  : ThreadedElementLoop<ConstElemRange>(x._fe_problem),
    _query(x._query),
    _query_subdomain(x._query_subdomain),
    _query_boundary(x._query_boundary),
    _aux_sys(x._aux_sys)
{
}

ComputeUserObjectsThread::~ComputeUserObjectsThread() {}

void
ComputeUserObjectsThread::subdomainChanged()
{
  // for the current thread get block objects for the current subdomain and *all* side objects
  std::vector<UserObject *> objs;
  querySubdomain(Interfaces::ElementUserObject | Interfaces::InternalSideUserObject |
                     Interfaces::InterfaceUserObject | Interfaces::DomainUserObject,
                 objs);

  _query.clone()
      .condition<AttribThread>(_tid)
      .condition<AttribInterfaces>(Interfaces::DomainUserObject)
      .queryInto(_all_domain_objs);

  std::vector<UserObject *> side_objs;
  _query.clone()
      .condition<AttribThread>(_tid)
      .condition<AttribInterfaces>(Interfaces::SideUserObject)
      .queryInto(side_objs);

  objs.insert(objs.begin(), side_objs.begin(), side_objs.end());

  // collect dependencies and run subdomain setup
  _fe_problem.subdomainSetup(_subdomain, _tid);

  std::set<MooseVariableFEBase *> needed_moose_vars;
  std::unordered_set<unsigned int> needed_mat_props;
  std::set<TagID> needed_fe_var_vector_tags;
  for (const auto obj : objs)
  {
    auto v_obj = dynamic_cast<MooseVariableDependencyInterface *>(obj);
    if (v_obj)
    {
      const auto & v_deps = v_obj->getMooseVariableDependencies();
      needed_moose_vars.insert(v_deps.begin(), v_deps.end());
    }

    auto m_obj = dynamic_cast<MaterialPropertyInterface *>(obj);
    if (m_obj)
    {
      auto & m_deps = m_obj->getMatPropDependencies();
      needed_mat_props.insert(m_deps.begin(), m_deps.end());
    }

    auto c_obj = dynamic_cast<Coupleable *>(obj);
    if (c_obj)
    {
      const auto & tag_deps = c_obj->getFEVariableCoupleableVectorTags();
      needed_fe_var_vector_tags.insert(tag_deps.begin(), tag_deps.end());
    }

    obj->subdomainSetup();
  }
  _fe_problem.getMaterialWarehouse().updateBlockFEVariableCoupledVectorTagDependency(
      _subdomain, needed_fe_var_vector_tags, _tid);

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.setActiveFEVariableCoupleableVectorTags(needed_fe_var_vector_tags, _tid);
  _fe_problem.prepareMaterials(needed_mat_props, _subdomain, _tid);

  querySubdomain(Interfaces::InternalSideUserObject, _internal_side_objs);
  querySubdomain(Interfaces::ElementUserObject, _element_objs);
  querySubdomain(Interfaces::ShapeElementUserObject, _shape_element_objs);
  querySubdomain(Interfaces::DomainUserObject, _domain_objs);
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
  {
    uo->execute();

    // update the aux solution vector if writable coupled variables are used
    if (uo->hasWritableCoupledVariables())
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (auto * var : uo->getWritableCoupledVariables())
        var->insert(_aux_sys.solution());
    }
  }

  for (auto & uo : _domain_objs)
  {
    uo->preExecuteOnElement();
    uo->executeOnElement();
  }

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
  if (userobjs.size() == 0 && _domain_objs.size() == 0)
    return;

  _fe_problem.reinitElemFace(elem, side, _tid);

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

  for (auto & uo : _domain_objs)
  {
    uo->preExecuteOnBoundary();
    uo->executeOnBoundary();
  }

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

  if (_internal_side_objs.size() == 0 && _domain_objs.size() == 0)
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

  for (auto & uo : _domain_objs)
    if (!uo->blockRestricted() || uo->hasBlocks(neighbor->subdomain_id()))
    {
      uo->preExecuteOnInternalSide();
      uo->executeOnInternalSide();
    }
}

void
ComputeUserObjectsThread::onExternalSide(const Elem * elem, unsigned int side)
{
  // We are not initializing any materials here because objects that perform calculations should
  // run onBoundary. onExternalSide should be used for mesh updates (e.g. adding/removing
  // boundaries). Note that _current_elem / _current_side are not getting updated either.
  for (auto & uo : _domain_objs)
    uo->executeOnExternalSide(elem, side);
}

void
ComputeUserObjectsThread::onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id)
{
  // Pointer to the neighbor we are currently working on.
  const Elem * neighbor = elem->neighbor_ptr(side);
  if (!(neighbor->active()))
    return;

  std::vector<UserObject *> interface_objs;
  queryBoundary(Interfaces::InterfaceUserObject, bnd_id, interface_objs);

  bool has_domain_objs = false;
  // we need to check all domain user objects because a domain user object may not be active
  // on the current subdomain but should be executed on the interface that it attaches to
  for (const auto * const domain_uo : _all_domain_objs)
    if (domain_uo->shouldExecuteOnInterface())
    {
      has_domain_objs = true;
      break;
    }

  // if we do not have any interface user objects and domain user objects on the current
  // interface
  if (interface_objs.empty() && !has_domain_objs)
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

  for (const auto & uo : interface_objs)
    uo->execute();

  for (auto & uo : _all_domain_objs)
    if (uo->shouldExecuteOnInterface())
    {
      uo->preExecuteOnInterface();
      uo->executeOnInterface();
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

void
ComputeUserObjectsThread::printGeneralExecutionInformation() const
{
  if (_fe_problem.shouldPrintExecution(_tid))
  {
    const auto & console = _fe_problem.console();
    const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
    console << "[DBG] Computing elemental user objects on " << execute_on << std::endl;
    mooseDoOnce(console << "[DBG] Execution order of objects types on each element then its sides:"
                        << std::endl;
                // onElement
                console << "[DBG] - element user objects" << std::endl;
                console << "[DBG] - domain user objects" << std::endl;
                console << "[DBG] - element user objects contributing to the Jacobian" << std::endl;

                // onBoundary
                console << "[DBG] - side user objects" << std::endl;
                console << "[DBG] - domain user objects executing on sides" << std::endl;
                console << "[DBG] - side user objects contributing to the Jacobian" << std::endl;

                // onInternalSide
                console << "[DBG] - internal side user objects" << std::endl;
                console << "[DBG] - domain user objects executing on internal sides" << std::endl;

                // onInterface
                console << "[DBG] - interface user objects" << std::endl;
                console << "[DBG] - domain user objects executing at interfaces" << std::endl;);
  }
}

void
ComputeUserObjectsThread::printBlockExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid))
    return;

  // Gather all user objects that may execute
  // TODO: restrict this gathering of boundary objects to boundaries that are present
  // in the current block
  std::vector<ShapeSideUserObject *> shapers;
  const_cast<ComputeUserObjectsThread *>(this)->queryBoundary(
      Interfaces::ShapeSideUserObject, Moose::ANY_BOUNDARY_ID, shapers);

  std::vector<SideUserObject *> side_uos;
  const_cast<ComputeUserObjectsThread *>(this)->queryBoundary(
      Interfaces::SideUserObject, Moose::ANY_BOUNDARY_ID, side_uos);

  std::vector<InterfaceUserObject *> interface_objs;
  const_cast<ComputeUserObjectsThread *>(this)->queryBoundary(
      Interfaces::InterfaceUserObject, Moose::ANY_BOUNDARY_ID, interface_objs);

  std::vector<const DomainUserObject *> domain_interface_uos;
  for (const auto * const domain_uo : _domain_objs)
    if (domain_uo->shouldExecuteOnInterface())
      domain_interface_uos.push_back(domain_uo);

  // Approximation of the number of user objects currently executing
  const auto num_objects = _element_objs.size() + _domain_objs.size() + _shape_element_objs.size() +
                           side_uos.size() + shapers.size() + _internal_side_objs.size() +
                           interface_objs.size() + domain_interface_uos.size();

  const auto & console = _fe_problem.console();
  const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();

  if (num_objects > 0)
  {
    if (_blocks_exec_printed.count(_subdomain))
      return;

    console << "[DBG] Ordering of User Objects on block " << _subdomain << std::endl;
    // Output specific ordering of objects
    printExecutionOrdering<ElementUserObject>(_element_objs, "element user objects");
    printExecutionOrdering<DomainUserObject>(_domain_objs, "domain user objects");
    if (_fe_problem.currentlyComputingJacobian())
      printExecutionOrdering<ShapeElementUserObject>(
          _shape_element_objs, "element user objects contributing to the Jacobian");
    printExecutionOrdering<SideUserObject>(side_uos, "side user objects");
    if (_fe_problem.currentlyComputingJacobian())
      printExecutionOrdering<ShapeSideUserObject>(shapers,
                                                  "side user objects contributing to the Jacobian");
    printExecutionOrdering<InternalSideUserObject>(_internal_side_objs,
                                                   "internal side user objects");
    printExecutionOrdering<InterfaceUserObject>(interface_objs, "interface user objects");
    console << "[DBG] Only user objects active on local element/sides are executed" << std::endl;
  }
  else if (num_objects == 0 && !_blocks_exec_printed.count(_subdomain))
    console << "[DBG] No User Objects on block " << _subdomain << " on " << execute_on.name()
            << std::endl;

  // Mark subdomain as having printed to avoid printing again
  _blocks_exec_printed.insert(_subdomain);
}
