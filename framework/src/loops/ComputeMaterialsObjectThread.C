//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ComputeMaterialsObjectThread.h"
#include "NonlinearSystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "MaterialPropertyStorage.h"
#include "MaterialData.h"
#include "Assembly.h"
#include "AuxKernel.h"
#include "Material.h"

#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

ComputeMaterialsObjectThread::ComputeMaterialsObjectThread(
    FEProblemBase & fe_problem,
    MaterialPropertyStorage & material_props,
    MaterialPropertyStorage & bnd_material_props,
    MaterialPropertyStorage & neighbor_material_props,
    std::vector<std::vector<std::unique_ptr<Assembly>>> & assembly)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _fe_problem(fe_problem),
    _material_props(material_props),
    _bnd_material_props(bnd_material_props),
    _neighbor_material_props(neighbor_material_props),
    _materials(_fe_problem.getRegularMaterialsWarehouse()),
    _interface_materials(_fe_problem.getInterfaceMaterialsWarehouse()),
    _discrete_materials(_fe_problem.getDiscreteMaterialWarehouse()),
    _assembly(assembly),
    _need_internal_side_material(false),
    _has_stateful_props(_material_props.hasStatefulProperties()),
    _has_bnd_stateful_props(_bnd_material_props.hasStatefulProperties()),
    _has_neighbor_stateful_props(_neighbor_material_props.hasStatefulProperties())
{
}

// Splitting Constructor
ComputeMaterialsObjectThread::ComputeMaterialsObjectThread(ComputeMaterialsObjectThread & x,
                                                           Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _fe_problem(x._fe_problem),
    _material_props(x._material_props),
    _bnd_material_props(x._bnd_material_props),
    _neighbor_material_props(x._neighbor_material_props),
    _materials(x._materials),
    _interface_materials(x._interface_materials),
    _discrete_materials(x._discrete_materials),
    _assembly(x._assembly),
    _need_internal_side_material(x._need_internal_side_material),
    _has_stateful_props(_material_props.hasStatefulProperties()),
    _has_bnd_stateful_props(_bnd_material_props.hasStatefulProperties()),
    _has_neighbor_stateful_props(_neighbor_material_props.hasStatefulProperties())
{
}

void
ComputeMaterialsObjectThread::subdomainChanged()
{
  _need_internal_side_material = _fe_problem.needSubdomainMaterialOnSide(_subdomain, _tid);
  _fe_problem.subdomainSetup(_subdomain, _tid);

  std::set<MooseVariableFEBase *> needed_moose_vars;
  _materials.updateVariableDependency(needed_moose_vars, _tid);
  _discrete_materials.updateVariableDependency(needed_moose_vars, _tid);
  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);

  std::set<TagID> needed_fe_var_vector_tags;
  _materials.updateBlockFEVariableCoupledVectorTagDependency(
      _subdomain, needed_fe_var_vector_tags, _tid);
  _discrete_materials.updateBlockFEVariableCoupledVectorTagDependency(
      _subdomain, needed_fe_var_vector_tags, _tid);
  _fe_problem.setActiveFEVariableCoupleableVectorTags(needed_fe_var_vector_tags, _tid);
}

void
ComputeMaterialsObjectThread::onElement(const Elem * elem)
{
  if (_materials.hasActiveBlockObjects(_subdomain, _tid) ||
      _discrete_materials.hasActiveBlockObjects(_subdomain, _tid))
  {
    _fe_problem.prepare(elem, _tid);
    _fe_problem.reinitElem(elem, _tid);

    auto & material_data = _material_props.getMaterialData(_tid);

    unsigned int n_points = _assembly[_tid][0]->qRule()->n_points();
    material_data.resize(n_points);

    if (_has_stateful_props)
    {
      if (_discrete_materials.hasActiveBlockObjects(_subdomain, _tid))
        _material_props.initStatefulProps(
            _tid, _discrete_materials.getActiveBlockObjects(_subdomain, _tid), n_points, *elem);
      if (_materials.hasActiveBlockObjects(_subdomain, _tid))
        _material_props.initStatefulProps(
            _tid, _materials.getActiveBlockObjects(_subdomain, _tid), n_points, *elem);
    }
  }
}

void
ComputeMaterialsObjectThread::onBoundary(const Elem * elem,
                                         unsigned int side,
                                         BoundaryID bnd_id,
                                         const Elem * /*lower_d_elem = nullptr*/)
{
  if (_fe_problem.needBoundaryMaterialOnSide(bnd_id, _tid))
  {
    _fe_problem.reinitElemFace(elem, side, _tid);
    unsigned int face_n_points = _assembly[_tid][0]->qRuleFace()->n_points();

    _bnd_material_props.getMaterialData(_tid).resize(face_n_points);

    if (_has_bnd_stateful_props)
    {
      // Face Materials
      if (_discrete_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(_subdomain, _tid))
        _bnd_material_props.initStatefulProps(
            _tid,
            _discrete_materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(_subdomain, _tid),
            face_n_points,
            *elem,
            side);
      if (_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(_subdomain, _tid))
        _bnd_material_props.initStatefulProps(
            _tid,
            _materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(_subdomain, _tid),
            face_n_points,
            *elem,
            side);

      // Boundary Materials
      if (_discrete_materials.hasActiveBoundaryObjects(bnd_id, _tid))
        _bnd_material_props.initStatefulProps(
            _tid, _materials.getActiveBoundaryObjects(bnd_id, _tid), face_n_points, *elem, side);
      if (_materials.hasActiveBoundaryObjects(bnd_id, _tid))
        _bnd_material_props.initStatefulProps(
            _tid, _materials.getActiveBoundaryObjects(bnd_id, _tid), face_n_points, *elem, side);
    }
  }
}

void
ComputeMaterialsObjectThread::onInternalSide(const Elem * elem, unsigned int side)
{
  if (_need_internal_side_material)
  {
    const Elem * neighbor = elem->neighbor_ptr(side);

    _fe_problem.reinitElemNeighborAndLowerD(elem, side, _tid);
    unsigned int face_n_points = _assembly[_tid][0]->qRuleFace()->n_points();
    _bnd_material_props.getMaterialData(_tid).resize(face_n_points);
    _neighbor_material_props.getMaterialData(_tid).resize(face_n_points);

    if (_has_bnd_stateful_props)
    {
      if (_discrete_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(_subdomain, _tid))
        _bnd_material_props.initStatefulProps(
            _tid,
            _discrete_materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(_subdomain, _tid),
            face_n_points,
            *elem,
            side);
      if (_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(_subdomain, _tid))
        _bnd_material_props.initStatefulProps(
            _tid,
            _materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(_subdomain, _tid),
            face_n_points,
            *elem,
            side);
    }

    unsigned int neighbor_side = neighbor->which_neighbor_am_i(_assembly[_tid][0]->elem());

    if (_has_neighbor_stateful_props)
    {
      // Neighbor Materials
      if (_discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(
              neighbor->subdomain_id(), _tid))
        _neighbor_material_props.initStatefulProps(
            _tid,
            _discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(
                neighbor->subdomain_id(), _tid),
            face_n_points,
            *elem,
            side);
      if (_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(neighbor->subdomain_id(),
                                                                          _tid))
        _neighbor_material_props.initStatefulProps(
            _tid,
            _materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(
                neighbor->subdomain_id(), _tid),
            face_n_points,
            *neighbor,
            neighbor_side);
    }
  }
}

void
ComputeMaterialsObjectThread::onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id)
{
  if (!_fe_problem.needInterfaceMaterialOnSide(bnd_id, _tid))
    return;

  _fe_problem.reinitElemFace(elem, side, _tid);
  unsigned int face_n_points = _assembly[_tid][0]->qRuleFace()->n_points();

  _bnd_material_props.getMaterialData(_tid).resize(face_n_points);
  _neighbor_material_props.getMaterialData(_tid).resize(face_n_points);

  if (_has_bnd_stateful_props)
  {
    // Face Materials
    if (_discrete_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(_subdomain, _tid))
      _bnd_material_props.initStatefulProps(
          _tid,
          _discrete_materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(_subdomain, _tid),
          face_n_points,
          *elem,
          side);

    if (_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(_subdomain, _tid))
      _bnd_material_props.initStatefulProps(
          _tid,
          _materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(_subdomain, _tid),
          face_n_points,
          *elem,
          side);

    // Boundary Materials
    if (_discrete_materials.hasActiveBoundaryObjects(bnd_id, _tid))
      _bnd_material_props.initStatefulProps(
          _tid, _materials.getActiveBoundaryObjects(bnd_id, _tid), face_n_points, *elem, side);

    if (_materials.hasActiveBoundaryObjects(bnd_id, _tid))
      _bnd_material_props.initStatefulProps(
          _tid, _materials.getActiveBoundaryObjects(bnd_id, _tid), face_n_points, *elem, side);
  }

  const Elem * neighbor = elem->neighbor_ptr(side);
  unsigned int neighbor_side = neighbor->which_neighbor_am_i(_assembly[_tid][0]->elem());

  // Do we have neighbor stateful properties or do we have stateful interface material properties?
  // If either then we need to reinit the neighbor, so at least at a minimum _neighbor_elem isn't
  // NULL!
  if (neighbor->active() &&
      (_has_neighbor_stateful_props ||
       (_has_bnd_stateful_props && _interface_materials.hasActiveBoundaryObjects(bnd_id, _tid))))
    _fe_problem.reinitNeighbor(elem, side, _tid);

  if (_has_neighbor_stateful_props && neighbor->active())
  {
    // Neighbor Materials
    if (_discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(
            neighbor->subdomain_id(), _tid))
      _neighbor_material_props.initStatefulProps(
          _tid,
          _discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(
              neighbor->subdomain_id(), _tid),
          face_n_points,
          *elem,
          side);

    if (_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(neighbor->subdomain_id(),
                                                                        _tid))
      _neighbor_material_props.initStatefulProps(
          _tid,
          _materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(neighbor->subdomain_id(),
                                                                          _tid),
          face_n_points,
          *neighbor,
          neighbor_side);
  }

  // Interface Materials. Make sure we do these after neighbors
  if (_has_bnd_stateful_props && _interface_materials.hasActiveBoundaryObjects(bnd_id, _tid))
    _bnd_material_props.initStatefulProps(
        _tid,
        _interface_materials.getActiveBoundaryObjects(bnd_id, _tid),
        face_n_points,
        *elem,
        side);
}

void
ComputeMaterialsObjectThread::join(const ComputeMaterialsObjectThread & /*y*/)
{
}

void
ComputeMaterialsObjectThread::post()
{
  _fe_problem.clearActiveElementalMooseVariables(_tid);
}
