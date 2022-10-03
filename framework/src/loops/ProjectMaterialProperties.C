//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ProjectMaterialProperties.h"
#include "Problem.h"
#include "FEProblem.h"
#include "MaterialPropertyStorage.h"
#include "MaterialData.h"
#include "Assembly.h"
#include "AuxKernel.h"

#include "libmesh/threads.h"
#include "libmesh/elem.h"

ProjectMaterialProperties::ProjectMaterialProperties(
    bool refine,
    FEProblemBase & fe_problem,
    std::vector<std::shared_ptr<MaterialData>> & material_data,
    std::vector<std::shared_ptr<MaterialData>> & bnd_material_data,
    MaterialPropertyStorage & material_props,
    MaterialPropertyStorage & bnd_material_props,
    std::vector<std::vector<std::unique_ptr<Assembly>>> & assembly)
  : ThreadedElementLoop<ConstElemPointerRange>(fe_problem),
    _refine(refine),
    _fe_problem(fe_problem),
    _material_data(material_data),
    _bnd_material_data(bnd_material_data),
    _material_props(material_props),
    _bnd_material_props(bnd_material_props),
    _assembly(assembly),
    _need_internal_side_material(false),
    _materials(_fe_problem.getRegularMaterialsWarehouse()),
    _discrete_materials(_fe_problem.getDiscreteMaterialWarehouse())
{
}

// Splitting Constructor
ProjectMaterialProperties::ProjectMaterialProperties(ProjectMaterialProperties & x,
                                                     Threads::split split)
  : ThreadedElementLoop<ConstElemPointerRange>(x, split),
    _refine(x._refine),
    _fe_problem(x._fe_problem),
    _material_data(x._material_data),
    _bnd_material_data(x._bnd_material_data),
    _material_props(x._material_props),
    _bnd_material_props(x._bnd_material_props),
    _assembly(x._assembly),
    _need_internal_side_material(x._need_internal_side_material),
    _materials(x._materials),
    _discrete_materials(x._discrete_materials)
{
}

ProjectMaterialProperties::~ProjectMaterialProperties() {}

void
ProjectMaterialProperties::subdomainChanged()
{
  _need_internal_side_material = _fe_problem.needSubdomainMaterialOnSide(_subdomain, _tid);
}

void
ProjectMaterialProperties::onElement(const Elem * elem)
{
  // This check mirrors the check in ComputeMaterialsObjectThread::onElement as it must because it
  // is possible that there are no materials on this element's subdomain, e.g. if we are doing
  // mortar, in which case the properties will not have been resized in
  // ComputeMaterialsObjectThread::onElement, and consequently if we were to proceed we would get
  // bad access errors
  if (!_materials.hasActiveBlockObjects(elem->subdomain_id(), _tid) &&
      !_discrete_materials.hasActiveBlockObjects(elem->subdomain_id(), _tid))
    return;

  _assembly[_tid][0]->reinit(elem);

  if (_refine)
  {
    const std::vector<std::vector<QpMap>> & refinement_map =
        _mesh.getRefinementMap(*elem, -1, -1, -1);

    _material_props.prolongStatefulProps(
        refinement_map,
        *_assembly[_tid][0]->qRule(),
        *_assembly[_tid][0]->qRuleFace(),
        _material_props, // Passing in the same properties to do volume to volume projection
        *_material_data[_tid],
        *elem,
        -1,
        -1,
        -1); // Gets us volume projection
  }
  else
  {
    const std::vector<std::pair<unsigned int, QpMap>> & coarsening_map =
        _mesh.getCoarseningMap(*elem, -1);

    _material_props.restrictStatefulProps(coarsening_map,
                                          _mesh.coarsenedElementChildren(elem),
                                          *_assembly[_tid][0]->qRule(),
                                          *_assembly[_tid][0]->qRuleFace(),
                                          *_material_data[_tid],
                                          *elem,
                                          -1);
  }
}

void
ProjectMaterialProperties::onBoundary(const Elem * elem,
                                      unsigned int side,
                                      BoundaryID bnd_id,
                                      const Elem * /*lower_d_elem = nullptr*/)
{
  if (_fe_problem.needBoundaryMaterialOnSide(bnd_id, _tid) &&
      _bnd_material_props.hasStatefulProperties())
  {
    _assembly[_tid][0]->reinit(elem, side);

    if (_refine)
    {
      const std::vector<std::vector<QpMap>> & refinement_map =
          _mesh.getRefinementMap(*elem, side, -1, side);

      _bnd_material_props.prolongStatefulProps(
          refinement_map,
          *_assembly[_tid][0]->qRule(),
          *_assembly[_tid][0]->qRuleFace(),
          _bnd_material_props, // Passing in the same properties to do side_to_side projection
          *_bnd_material_data[_tid],
          *elem,
          side,
          -1,
          side); // Gets us side to side projection
    }
    else
    {
      const std::vector<std::pair<unsigned int, QpMap>> & coarsening_map =
          _mesh.getCoarseningMap(*elem, side);

      _bnd_material_props.restrictStatefulProps(coarsening_map,
                                                _mesh.coarsenedElementChildren(elem),
                                                *_assembly[_tid][0]->qRule(),
                                                *_assembly[_tid][0]->qRuleFace(),
                                                *_material_data[_tid],
                                                *elem,
                                                side);
    }
  }
}

void
ProjectMaterialProperties::onInternalSide(const Elem * elem, unsigned int /*side*/)
{
  if (_need_internal_side_material &&
      _refine) // If we're refining then we need to also project "internal" child sides.
  {
    for (unsigned int child = 0; child < elem->n_children(); child++)
    {
      const Elem * child_elem = elem->child_ptr(child);

      for (unsigned int side = 0; side < child_elem->n_sides(); side++)
      {
        if (!elem->is_child_on_side(child, side)) // Otherwise we already projected it
        {
          const std::vector<std::vector<QpMap>> & refinement_map =
              _mesh.getRefinementMap(*elem, -1, child, side);

          _bnd_material_props.prolongStatefulProps(
              refinement_map,
              *_assembly[_tid][0]->qRule(),
              *_assembly[_tid][0]->qRuleFace(),
              _material_props, // Passing in the same properties to do side_to_side projection
              *_bnd_material_data[_tid],
              *elem,
              -1,
              child,
              side); // Gets us volume to side projection
        }
      }
    }
  }
}

void
ProjectMaterialProperties::join(const ProjectMaterialProperties & /*y*/)
{
}
