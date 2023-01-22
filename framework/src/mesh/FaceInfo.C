//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FaceInfo.h"
#include "MooseTypes.h"

#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/fe_base.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/remote_elem.h"

FaceInfo::FaceInfo(const ElemInfo * elem_info, unsigned int side, const dof_id_type id)
  : _elem_info(elem_info),
    _neighbor_info(nullptr),
    _id(id),
    _processor_id(_elem_info->elem()->processor_id()),
    _elem_side_id(side),
    _neighbor_side_id(libMesh::invalid_uint),
    _gc(0.5)
{
  // Compute face-related quantities
  const std::unique_ptr<const Elem> face = _elem_info->elem()->build_side_ptr(_elem_side_id);
  _face_area = face->volume();
  _face_centroid = face->vertex_average();

  // Compute the face-normals
  unsigned int dim = _elem_info->elem()->dim();
  const auto r_cf = _face_centroid - _elem_info->centroid();

  // For 1D elements, this is simple
  if (dim == 1)
    _normal = r_cf / r_cf.norm();
  // For 2D elements, this is equally simple, we just need to make sure that
  // the normal points in the right direction.
  else if (dim == 2)
  {
    Point side = face->node_ref(0) - face->node_ref(1);
    _normal = Point(-side(1), side(0));
    _normal /= _normal.norm();
    if (_normal * r_cf < 0.0)
      _normal *= -1.0;
  }
  // In 3D we need to use the vector product
  else
  {
    Point side_1 = face->node_ref(0) - face->node_ref(1);
    Point side_2 = face->node_ref(0) - face->node_ref(2);
    _normal = side_1.cross(side_2);
    _normal /= _normal.norm();
    if (_normal * r_cf < 0.0)
      _normal *= -1.0;
  }
}

void
FaceInfo::computeInternalCoefficients(const ElemInfo * const neighbor_info)
{
  mooseAssert(neighbor_info,
              "We need a neighbor if we want to compute interpolation coefficients!");
  _neighbor_info = neighbor_info;
  _neighbor_side_id = _neighbor_info->elem()->which_neighbor_am_i(_elem_info->elem());

  // Setup quantities used for the approximation of the spatial derivatives
  _d_cn = _neighbor_info->centroid() - _elem_info->centroid();
  _d_cn_mag = _d_cn.norm();
  _e_cn = _d_cn / _d_cn_mag;

  Point r_intersection =
      _elem_info->centroid() +
      (((_face_centroid - _elem_info->centroid()) * _normal) / (_e_cn * _normal)) * _e_cn;

  // For interpolation coefficients
  _gc = (_neighbor_info->centroid() - r_intersection).norm() / _d_cn_mag;
}

void
FaceInfo::computeBoundaryCoefficients()
{
  mooseAssert(!_neighbor_info, "This functions shall only be called on a boundary!");

  // Setup quantities used for the approximation of the spatial derivatives
  _d_cn = _face_centroid - _elem_info->centroid();
  _d_cn_mag = _d_cn.norm();
  _e_cn = _d_cn / _d_cn_mag;

  // For interpolation coefficients
  _gc = 1.0;
}

Point
FaceInfo::skewnessCorrectionVector() const
{
  const Point r_intersection =
      _elem_info->centroid() +
      (((_face_centroid - _elem_info->centroid()) * _normal) / (_e_cn * _normal)) * _e_cn;

  return _face_centroid - r_intersection;
}
