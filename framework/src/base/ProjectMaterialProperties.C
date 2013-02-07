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
#include "ProjectMaterialProperties.h"

#include "NonlinearSystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "MaterialWarehouse.h"
#include "MaterialPropertyStorage.h"
#include "MaterialData.h"
#include "Assembly.h"
#include "AuxKernel.h"

// libmesh includes
#include "libmesh/threads.h"
#include "libmesh/elem.h"

ProjectMaterialProperties::ProjectMaterialProperties(bool refine,
                                                     FEProblem & fe_problem, NonlinearSystem & sys, std::vector<MaterialData *> & material_data,
                                                     std::vector<MaterialData *> & bnd_material_data, MaterialPropertyStorage & material_props,
                                                     MaterialPropertyStorage & bnd_material_props, std::vector<MaterialWarehouse> & materials,
                                                     std::vector<Assembly *> & assembly) :
    ThreadedElementLoop<ConstElemPointerRange>(fe_problem, sys),
    _refine(refine),
    _fe_problem(fe_problem),
    _sys(sys),
    _material_data(material_data),
    _bnd_material_data(bnd_material_data),
    _material_props(material_props),
    _bnd_material_props(bnd_material_props),
    _materials(materials),
    _assembly(assembly)
{
}

// Splitting Constructor
ProjectMaterialProperties::ProjectMaterialProperties(ProjectMaterialProperties & x, Threads::split split) :
    ThreadedElementLoop<ConstElemPointerRange>(x, split),
    _refine(x._refine),
    _fe_problem(x._fe_problem),
    _sys(x._sys),
    _material_data(x._material_data),
    _bnd_material_data(x._bnd_material_data),
    _material_props(x._material_props),
    _bnd_material_props(x._bnd_material_props),
    _materials(x._materials),
    _assembly(x._assembly)
{
}

ProjectMaterialProperties::~ProjectMaterialProperties()
{
}

void
ProjectMaterialProperties::subdomainChanged()
{
}

void
ProjectMaterialProperties::onElement(const Elem *elem)
{
  _assembly[_tid]->reinit(elem);

  if(_refine)
  {
    const std::vector<std::vector<QpMap> > & refinement_map  = _mesh.getRefinementMap(*elem, -1, -1, -1);

    _material_props.prolongStatefulProps(refinement_map,
                                         *_assembly[_tid]->qRule(),
                                         *_assembly[_tid]->qRuleFace(),
                                         _material_props, // Passing in the same properties to do volume to volume projection
                                         *_material_data[_tid],
                                         *elem,
                                          -1,-1,-1); // Gets us volume projection
  }
  else
  {
    const std::vector<std::pair<unsigned int, QpMap> > & coarsening_map = _mesh.getCoarseningMap(*elem, -1);

    _material_props.restrictStatefulProps(coarsening_map,
                                          _mesh.coarsenedElementChildren(elem),
                                          *_assembly[_tid]->qRule(),
                                          *_assembly[_tid]->qRuleFace(),
                                          *_material_data[_tid],
                                          *elem,
                                          -1);
  }

  // Project the sides that correspond to sides of the parent element
  for (unsigned int side=0; side<elem->n_sides(); side++)
  {
    mooseAssert(_materials[_tid].hasFaceMaterials(_subdomain), "No face materials on subdomain block");
    _assembly[_tid]->reinit(elem, side);

    if(_refine)
    {
      const std::vector<std::vector<QpMap> > & refinement_map  = _mesh.getRefinementMap(*elem, side, -1, side);

      _bnd_material_props.prolongStatefulProps(refinement_map,
                                           *_assembly[_tid]->qRule(),
                                           *_assembly[_tid]->qRuleFace(),
                                           _bnd_material_props, // Passing in the same properties to do side_to_side projection
                                           *_bnd_material_data[_tid],
                                           *elem,
                                           side,-1,side); // Gets us side to side projection
    }
    else
    {
      const std::vector<std::pair<unsigned int, QpMap> > & coarsening_map = _mesh.getCoarseningMap(*elem, side);

      _bnd_material_props.restrictStatefulProps(coarsening_map,
                                                _mesh.coarsenedElementChildren(elem),
                                                *_assembly[_tid]->qRule(),
                                                *_assembly[_tid]->qRuleFace(),
                                                *_material_data[_tid],
                                                *elem, side);
    }
  }

  if(_refine) // If we're refining then we need to also project "internal" child sides.
  {
    for(unsigned int child=0; child<elem->n_children(); child++)
    {
      Elem * child_elem = elem->child(child);

      for(unsigned int side=0; side<child_elem->n_sides(); side++)
      {
        if(!elem->is_child_on_side(child, side)) // Otherwise we already projected it
        {
          const std::vector<std::vector<QpMap> > & refinement_map  = _mesh.getRefinementMap(*elem, -1, child, side);

          _bnd_material_props.prolongStatefulProps(refinement_map,
                                                   *_assembly[_tid]->qRule(),
                                                   *_assembly[_tid]->qRuleFace(),
                                                   _material_props, // Passing in the same properties to do side_to_side projection
                                                   *_bnd_material_data[_tid],
                                                   *elem,
                                                   -1,child,side); // Gets us volume to side projection
        }
      }
    }
  }
}

void
ProjectMaterialProperties::join(const ProjectMaterialProperties & /*y*/)
{
}
