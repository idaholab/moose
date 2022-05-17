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

FaceInfo::FaceInfo(const ElemInfo * elem_info, unsigned int side)
  : _elem_info(elem_info),
    _neighbor_info(nullptr),
    _processor_id(_elem_info->elem()->processor_id()),
    _id(std::make_pair(_elem_info->elem()->id(), side)),
    _elem_side_id(side),
    _face(const_cast<Elem *>(_elem_info->elem())->build_side_ptr(_elem_side_id)),
    _face_area(_face->volume()),
    _face_centroid(_face->vertex_average())
{
  // Compute the face-normals
  unsigned int dim = _elem_info->elem()->dim();
  Point vector_to_face = _face_centroid - _elem_info->centroid();

  // For 1D elements, this is simple
  if (dim == 1)
    _normal = vector_to_face / vector_to_face.norm();
  // For 2D elements, this is equally simple, we just need to make sure that
  // the normal points in the right direction.
  else if (dim == 2)
  {
    Point side = *_face->node_ptr(0) - *_face->node_ptr(1);
    _normal = Point(-side(1), side(0));
    _normal /= _normal.norm();
    if (_normal * vector_to_face < 0.0)
      _normal *= -1.0;
  }
  // In 3D we need to use the vector product
  else
  {
    Point side_1 = *_face->node_ptr(0) - *_face->node_ptr(1);
    Point side_2 = *_face->node_ptr(0) - *_face->node_ptr(2);
    _normal = side_1.cross(side_2);
    _normal /= _normal.norm();
    if (_normal * vector_to_face < 0.0)
      _normal *= -1.0;
  }
}

void
FaceInfo::computeCoefficients(const ElemInfo * const neighbor_info)
{
  // the neighbor info does not exist for domain boundaries. Additionally, we don't have any info
  // if the neighbor is a RemoteElem. This can happen for ghosted elements on the edge of a
  // stencil, for whom we have may have deleted some of their neighbors when running with a
  // distributed mesh
  _neighbor_info = neighbor_info;
  _valid_neighbor = _neighbor_info->elem() && _neighbor_info->elem() != remote_elem;
  _neighbor_side_id = _valid_neighbor
                          ? _neighbor_info->elem()->which_neighbor_am_i(_elem_info->elem())
                          : std::numeric_limits<unsigned int>::max();

  // Setup quantities used for the approximation of the spatial derivatives
  _d_cf = _neighbor_info->centroid() - _elem_info->centroid();
  _d_cf_mag = _d_cf.norm();
  _e_cf = _d_cf / _d_cf_mag;

  // Compute the position of the intersection of e_CF and the surface
  Point r_intersection =
      _elem_info->centroid() +
      (((_face_centroid - _elem_info->centroid()) * _normal) / (_e_cf * _normal)) * _e_cf;

  _skewness_correction_vector = _face_centroid - r_intersection;

  // For interpolation coefficients
  _gc = (_neighbor_info->centroid() - r_intersection).norm() / _d_cf_mag;
}

void
FaceInfo::computeCoefficients()
{
  _d_cf_mag = (_face_centroid - _elem_info->centroid()) * _normal;
  _d_cf = _d_cf_mag * _normal;
  _e_cf = _normal;

  _skewness_correction_vector = _face_centroid - (_elem_info->centroid() + _d_cf);
  _gc = 0.5;
}
