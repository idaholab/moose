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
    std::vector<std::shared_ptr<MaterialData>> & material_data,
    std::vector<std::shared_ptr<MaterialData>> & bnd_material_data,
    std::vector<std::shared_ptr<MaterialData>> & neighbor_material_data,
    MaterialPropertyStorage & material_props,
    MaterialPropertyStorage & bnd_material_props,
    MaterialPropertyStorage & neighbor_material_props,
    std::vector<std::unique_ptr<Assembly>> & assembly)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _fe_problem(fe_problem),
    _nl(fe_problem.getNonlinearSystemBase()),
    _material_data(material_data),
    _bnd_material_data(bnd_material_data),
    _neighbor_material_data(neighbor_material_data),
    _material_props(material_props),
    _bnd_material_props(bnd_material_props),
    _neighbor_material_props(neighbor_material_props),
    _materials(_fe_problem.getComputeMaterialWarehouse()),
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
    _nl(x._nl),
    _material_data(x._material_data),
    _bnd_material_data(x._bnd_material_data),
    _neighbor_material_data(x._neighbor_material_data),
    _material_props(x._material_props),
    _bnd_material_props(x._bnd_material_props),
    _neighbor_material_props(x._neighbor_material_props),
    _materials(x._materials),
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
  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
}

void
ComputeMaterialsObjectThread::onElement(const Elem * elem)
{
  if (_materials.hasActiveBlockObjects(_subdomain, _tid) ||
      _discrete_materials.hasActiveBlockObjects(_subdomain, _tid))
  {
    _fe_problem.prepare(elem, _tid);
    _fe_problem.reinitElem(elem, _tid);

    unsigned int n_points = _assembly[_tid]->qRule()->n_points();
    _material_data[_tid]->resize(n_points);

    if (_has_stateful_props)
    {
      if (_discrete_materials.hasActiveBlockObjects(_subdomain, _tid))
        _material_props.initStatefulProps(
            *_material_data[_tid],
            _discrete_materials.getActiveBlockObjects(_subdomain, _tid),
            n_points,
            *elem);
      if (_materials.hasActiveBlockObjects(_subdomain, _tid))
        _material_props.initStatefulProps(*_material_data[_tid],
                                          _materials.getActiveBlockObjects(_subdomain, _tid),
                                          n_points,
                                          *elem);
    }
  }
}

void
ComputeMaterialsObjectThread::onBoundary(const Elem * elem, unsigned int side, BoundaryID bnd_id)
{
  if (_fe_problem.needBoundaryMaterialOnSide(bnd_id, _tid))
  {
    _assembly[_tid]->reinit(elem, side);
    unsigned int face_n_points = _assembly[_tid]->qRuleFace()->n_points();

    _bnd_material_data[_tid]->resize(face_n_points);

    if (_has_bnd_stateful_props)
    {
      // Face Materials
      if (_discrete_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(_subdomain, _tid))
        _bnd_material_props.initStatefulProps(
            *_bnd_material_data[_tid],
            _discrete_materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(_subdomain, _tid),
            face_n_points,
            *elem,
            side);
      if (_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(_subdomain, _tid))
        _bnd_material_props.initStatefulProps(
            *_bnd_material_data[_tid],
            _materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(_subdomain, _tid),
            face_n_points,
            *elem,
            side);

      // Boundary Materials
      if (_discrete_materials.hasActiveBoundaryObjects(bnd_id, _tid))
        _bnd_material_props.initStatefulProps(*_bnd_material_data[_tid],
                                              _materials.getActiveBoundaryObjects(bnd_id, _tid),
                                              face_n_points,
                                              *elem,
                                              side);
      if (_materials.hasActiveBoundaryObjects(bnd_id, _tid))
        _bnd_material_props.initStatefulProps(*_bnd_material_data[_tid],
                                              _materials.getActiveBoundaryObjects(bnd_id, _tid),
                                              face_n_points,
                                              *elem,
                                              side);
    }
  }
}

void
ComputeMaterialsObjectThread::onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id)
{
  if (_fe_problem.needBoundaryMaterialOnInterface(bnd_id, _tid))
  {
    _assembly[_tid]->reinit(elem, side);
    unsigned int face_n_points = _assembly[_tid]->qRuleFace()->n_points();
    _bnd_material_data[_tid]->resize(face_n_points);
    _neighbor_material_data[_tid]->resize(face_n_points);

    if (_has_bnd_stateful_props)
    {
      // Face Materials
      if (_discrete_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(_subdomain, _tid))
        _bnd_material_props.initStatefulProps(
            *_bnd_material_data[_tid],
            _discrete_materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(_subdomain, _tid),
            face_n_points,
            *elem,
            side);

      if (_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(_subdomain, _tid))
        _bnd_material_props.initStatefulProps(
            *_bnd_material_data[_tid],
            _materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(_subdomain, _tid),
            face_n_points,
            *elem,
            side);

      // Boundary Materials
      if (_discrete_materials.hasActiveBoundaryObjects(bnd_id, _tid))
        _bnd_material_props.initStatefulProps(*_bnd_material_data[_tid],
                                              _materials.getActiveBoundaryObjects(bnd_id, _tid),
                                              face_n_points,
                                              *elem,
                                              side);

      if (_materials.hasActiveBoundaryObjects(bnd_id, _tid))
        _bnd_material_props.initStatefulProps(*_bnd_material_data[_tid],
                                              _materials.getActiveBoundaryObjects(bnd_id, _tid),
                                              face_n_points,
                                              *elem,
                                              side);
    }

    const Elem * neighbor = elem->neighbor_ptr(side);
    unsigned int neighbor_side = neighbor->which_neighbor_am_i(_assembly[_tid]->elem());
    const dof_id_type elem_id = elem->id(), neighbor_id = neighbor->id();

    if (_has_neighbor_stateful_props &&
        ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) ||
         (neighbor->level() < elem->level())))
    {
      _assembly[_tid]->reinitElemAndNeighbor(elem, side, neighbor, neighbor_side);

      // Neighbor Materials
      if (_discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(
              neighbor->subdomain_id(), _tid))
        _neighbor_material_props.initStatefulProps(
            *_bnd_material_data[_tid],
            _discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(
                neighbor->subdomain_id(), _tid),
            face_n_points,
            *elem,
            side);

      if (_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(neighbor->subdomain_id(),
                                                                          _tid))
        _neighbor_material_props.initStatefulProps(
            *_neighbor_material_data[_tid],
            _materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(
                neighbor->subdomain_id(), _tid),
            face_n_points,
            *neighbor,
            neighbor_side);
    }
  }
}

void
ComputeMaterialsObjectThread::onInternalSide(const Elem * elem, unsigned int side)
{
  if (_need_internal_side_material)
  {
    _assembly[_tid]->reinit(elem, side);
    unsigned int face_n_points = _assembly[_tid]->qRuleFace()->n_points();
    _bnd_material_data[_tid]->resize(face_n_points);
    _neighbor_material_data[_tid]->resize(face_n_points);

    if (_has_bnd_stateful_props)
    {
      if (_discrete_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(_subdomain, _tid))
        _bnd_material_props.initStatefulProps(
            *_bnd_material_data[_tid],
            _discrete_materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(_subdomain, _tid),
            face_n_points,
            *elem,
            side);
      if (_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(_subdomain, _tid))
        _bnd_material_props.initStatefulProps(
            *_bnd_material_data[_tid],
            _materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(_subdomain, _tid),
            face_n_points,
            *elem,
            side);
    }

    const Elem * neighbor = elem->neighbor_ptr(side);
    unsigned int neighbor_side = neighbor->which_neighbor_am_i(_assembly[_tid]->elem());
    const dof_id_type elem_id = elem->id(), neighbor_id = neighbor->id();

    if (_has_neighbor_stateful_props &&
        ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) ||
         (neighbor->level() < elem->level())))
    {
      _assembly[_tid]->reinitElemAndNeighbor(elem, side, neighbor, neighbor_side);

      // Neighbor Materials
      if (_discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(
              neighbor->subdomain_id(), _tid))
        _neighbor_material_props.initStatefulProps(
            *_bnd_material_data[_tid],
            _discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(
                neighbor->subdomain_id(), _tid),
            face_n_points,
            *elem,
            side);
      if (_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(neighbor->subdomain_id(),
                                                                          _tid))
        _neighbor_material_props.initStatefulProps(
            *_neighbor_material_data[_tid],
            _materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(
                neighbor->subdomain_id(), _tid),
            face_n_points,
            *neighbor,
            neighbor_side);
    }
  }
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
