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
    _processor_id(_elem_info->elem()->processor_id()),
    _id(std::make_pair(_elem_info->elem()->id(), side)),
    _elem_side_id(side),
    _face(const_cast<Elem *>(_elem_info->elem())->build_side_ptr(_elem_side_id)),
    _face_area(_face->volume()),
    _face_centroid(_face->vertex_average())
{
  // Compute the face-normals
  unsigned int dim = _elem_info->elem()->dim();
  // For 1D elements, this is simple
  if (dim == 1)
  {
    _normal = (_face_centroid - _elem_info->centroid());
    _normal /= _normal.norm();
  }
  // For 2D elements, this is equally simple, we just need to make sure that
  // the normal points in the right direction.
  else if (dim == 2)
  {
    std::vector<const Point *> side_nodes(2);
    for (const auto vertex_num : make_range(2))
      // We are casting the nodes to points in one go
      side_nodes[vertex_num] = _face->node_ptr(vertex_num);

    Point side = *side_nodes[0] - *side_nodes[1];
    _normal = Point(-side(1), side(0));
    _normal /= _normal.norm();
    if (_normal * (_face_centroid - _elem_info->centroid()) < 0.0)
      _normal *= -1.0;
  }
  // In 3D we need to use the vector product
  else
  {
    std::vector<const Point *> side_nodes(3);
    for (const auto vertex_num : make_range(3))
      // We are casting the nodes to points in one go
      side_nodes[vertex_num] = _face->node_ptr(vertex_num);

    Point side_1 = *side_nodes[0] - *side_nodes[1];
    Point side_2 = *side_nodes[0] - *side_nodes[2];
    _normal = side_1.cross(side_2);
    _normal /= _normal.norm();
    if (_normal * (_face_centroid - _elem_info->centroid()) < 0.0)
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

  // compute an centroid face normal by using 1-point quadrature
  unsigned int dim = _elem_info->elem()->dim();
  std::unique_ptr<FEBase> fe(FEBase::build(dim, FEType(_elem_info->elem()->default_order())));
  QGauss qface(dim - 1, CONSTANT);
  fe->attach_quadrature_rule(&qface);
  const std::vector<Point> & normals = fe->get_normals();
  fe->reinit(_elem_info->elem(), _elem_side_id);
  mooseAssert(normals.size() == 1, "FaceInfo construction broken w.r.t. computing face normals");
  _normal = normals[0];

  // Compute the position of the intersection of e_CF and the surface
  _r_intersection =
      _elem_info->centroid() +
      (((_face_centroid - _elem_info->centroid()) * _normal) / (_e_cf * _normal)) * _e_cf;

  // For interpolation coefficients
  _gc = (_neighbor_info->centroid() - _r_intersection).norm() / _d_cf_mag;
}
