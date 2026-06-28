//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GhostNodeFaceInterface.h"
#include "MooseApp.h"

// libMesh includes
#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"

#include <sstream>

registerMooseObject("MooseApp", GhostNodeFaceInterface);

using namespace libMesh;

InputParameters
GhostNodeFaceInterface::validParams()
{
  InputParameters params = RelationshipManager::validParams();
  params.addRequiredParam<BoundaryName>("primary_boundary",
                                        "The name of the primary boundary sideset.");
  params.addRequiredParam<BoundaryName>("secondary_boundary",
                                        "The name of the secondary boundary sideset.");
  params.addRequiredParam<bool>("enabled", "Whether this relationship manager should ghost.");
  params.addClassDescription(
      "Ghosts node-face constraint primary and secondary interface elements to processors that own "
      "secondary interface elements.");
  return params;
}

GhostNodeFaceInterface::GhostNodeFaceInterface(const InputParameters & params)
  : RelationshipManager(params),
    _primary_boundary_name(getParam<BoundaryName>("primary_boundary")),
    _secondary_boundary_name(getParam<BoundaryName>("secondary_boundary")),
    _enabled(getParam<bool>("enabled"))
{
}

GhostNodeFaceInterface::GhostNodeFaceInterface(const GhostNodeFaceInterface & other)
  : RelationshipManager(other),
    _primary_boundary_name(other._primary_boundary_name),
    _secondary_boundary_name(other._secondary_boundary_name),
    _enabled(other._enabled)
{
}

void
GhostNodeFaceInterface::internalInitWithMesh(const MeshBase &)
{
}

std::string
GhostNodeFaceInterface::getInfo() const
{
  std::ostringstream oss;
  oss << "GhostNodeFaceInterface" << (_enabled ? "" : " (disabled)");
  return oss.str();
}

bool
GhostNodeFaceInterface::hasSecondaryBoundaryFace(
    const MeshBase::const_element_iterator & range_begin,
    const MeshBase::const_element_iterator & range_end,
    const BoundaryID secondary_boundary_id,
    const bool generating_mesh) const
{
  const auto mesh_dim = _mesh->mesh_dimension();
  const BoundaryInfo & binfo = _mesh->get_boundary_info();

  for (const Elem * const elem : as_range(range_begin, range_end))
  {
    if (elem->dim() != mesh_dim)
      continue;

    if (generating_mesh)
    {
      if (elem->on_boundary())
        return true;
    }
    else
      for (const auto side : elem->side_index_range())
        if (binfo.has_boundary_id(elem, side, secondary_boundary_id))
          return true;
  }

  return false;
}

void
GhostNodeFaceInterface::operator()(const MeshBase::const_element_iterator & range_begin,
                                   const MeshBase::const_element_iterator & range_end,
                                   const processor_id_type p,
                                   map_type & coupled_elements)
{
  if (!_enabled)
    return;

  const bool generating_mesh = !_moose_mesh->getMeshPtr();
  const auto primary_boundary_id = generating_mesh
                                       ? Moose::INVALID_BOUNDARY_ID
                                       : _moose_mesh->getBoundaryID(_primary_boundary_name);
  const auto secondary_boundary_id = generating_mesh
                                         ? Moose::INVALID_BOUNDARY_ID
                                         : _moose_mesh->getBoundaryID(_secondary_boundary_name);

  if (!hasSecondaryBoundaryFace(range_begin, range_end, secondary_boundary_id, generating_mesh))
    return;

  const auto mesh_dim = _mesh->mesh_dimension();
  const BoundaryInfo & binfo = _mesh->get_boundary_info();

  for (const Elem * const elem : _mesh->active_element_ptr_range())
  {
    if (elem->processor_id() == p || elem->dim() != mesh_dim)
      continue;

    if (generating_mesh)
    {
      if (elem->on_boundary())
        coupled_elements.emplace(elem, _null_mat);
    }
    else
      for (const auto side : elem->side_index_range())
        if (binfo.has_boundary_id(elem, side, primary_boundary_id) ||
            binfo.has_boundary_id(elem, side, secondary_boundary_id))
        {
          coupled_elements.emplace(elem, _null_mat);
          break;
        }
  }
}

bool
GhostNodeFaceInterface::operator>=(const RelationshipManager & other) const
{
  const auto * const nfi = dynamic_cast<const GhostNodeFaceInterface *>(&other);
  if (!nfi || !baseGreaterEqual(*nfi))
    return false;

  if (_enabled && !nfi->_enabled)
    return true;

  if (!_enabled)
    return !nfi->_enabled;

  return nfi->_enabled && _use_displaced_mesh == nfi->_use_displaced_mesh &&
         _primary_boundary_name == nfi->_primary_boundary_name &&
         _secondary_boundary_name == nfi->_secondary_boundary_name;
}

std::unique_ptr<GhostingFunctor>
GhostNodeFaceInterface::clone() const
{
  return _app.getFactory().copyConstruct(*this);
}
