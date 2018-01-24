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
#include "NonlinearSystem.h"
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
    NonlinearSystemBase & sys,
    std::vector<std::shared_ptr<MaterialData>> & material_data,
    std::vector<std::shared_ptr<MaterialData>> & bnd_material_data,
    MaterialPropertyStorage & material_props,
    MaterialPropertyStorage & bnd_material_props,
    std::vector<Assembly *> & assembly)
  : ThreadedElementLoop<ConstElemPointerRange>(fe_problem),
    _refine(refine),
    _fe_problem(fe_problem),
    _sys(sys),
    _material_data(material_data),
    _bnd_material_data(bnd_material_data),
    _material_props(material_props),
    _bnd_material_props(bnd_material_props),
    _assembly(assembly),
    _need_internal_side_material(false)
{
}

// Splitting Constructor
ProjectMaterialProperties::ProjectMaterialProperties(ProjectMaterialProperties & x,
                                                     Threads::split split)
  : ThreadedElementLoop<ConstElemPointerRange>(x, split),
    _refine(x._refine),
    _fe_problem(x._fe_problem),
    _sys(x._sys),
    _material_data(x._material_data),
    _bnd_material_data(x._bnd_material_data),
    _material_props(x._material_props),
    _bnd_material_props(x._bnd_material_props),
    _assembly(x._assembly),
    _need_internal_side_material(x._need_internal_side_material)
{
}

ProjectMaterialProperties::~ProjectMaterialProperties() {}

void
ProjectMaterialProperties::subdomainChanged()
{
  _need_internal_side_material = _fe_problem.needMaterialOnSide(_subdomain, _tid);
}

void
ProjectMaterialProperties::onElement(const Elem * elem)
{
  _assembly[_tid]->reinit(elem);

  if (_refine)
  {
    const std::vector<std::vector<QpMap>> & refinement_map =
        _mesh.getRefinementMap(*elem, -1, -1, -1);

    _material_props.prolongStatefulProps(
        refinement_map,
        *_assembly[_tid]->qRule(),
        *_assembly[_tid]->qRuleFace(),
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
                                          *_assembly[_tid]->qRule(),
                                          *_assembly[_tid]->qRuleFace(),
                                          *_material_data[_tid],
                                          *elem,
                                          -1);
  }
}

void
ProjectMaterialProperties::onBoundary(const Elem * elem, unsigned int side, BoundaryID bnd_id)
{
  if (_fe_problem.needMaterialOnSide(bnd_id, _tid))
  {
    _assembly[_tid]->reinit(elem, side);

    if (_refine)
    {
      const std::vector<std::vector<QpMap>> & refinement_map =
          _mesh.getRefinementMap(*elem, side, -1, side);

      _bnd_material_props.prolongStatefulProps(
          refinement_map,
          *_assembly[_tid]->qRule(),
          *_assembly[_tid]->qRuleFace(),
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
                                                *_assembly[_tid]->qRule(),
                                                *_assembly[_tid]->qRuleFace(),
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
      Elem * child_elem = elem->child(child);

      for (unsigned int side = 0; side < child_elem->n_sides(); side++)
      {
        if (!elem->is_child_on_side(child, side)) // Otherwise we already projected it
        {
          const std::vector<std::vector<QpMap>> & refinement_map =
              _mesh.getRefinementMap(*elem, -1, child, side);

          _bnd_material_props.prolongStatefulProps(
              refinement_map,
              *_assembly[_tid]->qRule(),
              *_assembly[_tid]->qRuleFace(),
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
