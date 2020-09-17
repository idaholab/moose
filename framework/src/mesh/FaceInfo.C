//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FaceInfo.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/fe_base.h"
#include "libmesh/quadrature_gauss.h"

FaceInfo::FaceInfo(const Elem * elem, unsigned int side, const Elem * neighbor)
  : _processor_id(elem->processor_id())
{
  _elem = elem;
  _neighbor = neighbor;

  _elem_side_id = side;
  _elem_centroid = elem->centroid();
  _elem_volume = elem->volume();

  std::unique_ptr<const Elem> face = elem->build_side_ptr(_elem_side_id);
  _face_area = face->volume();
  _face_centroid = face->centroid();

  // 1. compute face centroid
  // 2. compute an centroid face normal by using 1-point quadrature
  //    meshes)
  unsigned int dim = elem->dim();
  std::unique_ptr<FEBase> fe(FEBase::build(dim, FEType(elem->default_order())));
  QGauss qface(dim - 1, CONSTANT);
  fe->attach_quadrature_rule(&qface);
  const std::vector<Point> & normals = fe->get_normals();
  fe->reinit(elem, _elem_side_id);
  mooseAssert(normals.size() == 1, "FaceInfo construction broken w.r.t. computing face normals");
  _normal = normals[0];

  // the neighbor info does not exist for domain boundaries
  if (!_neighbor)
  {
    _neighbor_side_id = std::numeric_limits<unsigned int>::max();
    _neighbor_centroid = 2 * (_face_centroid - _elem_centroid) + _elem_centroid;
    _neighbor_volume = _elem_volume;
  }
  else
  {
    _neighbor_side_id = neighbor->which_neighbor_am_i(elem);
    _neighbor_centroid = neighbor->centroid();
    _neighbor_volume = neighbor->volume();
  }

  _gc = (_neighbor_centroid - _face_centroid).norm() /
        ((_neighbor_centroid - _face_centroid).norm() + (_elem_centroid - _face_centroid).norm());

  _d_cf = _neighbor_centroid - _elem_centroid;
  _d_cf_mag = _d_cf.norm();
  _e_cf = _d_cf / _d_cf_mag;

  _vertices.resize(face->n_vertices());
  for (const auto vertex_num : make_range(face->n_vertices()))
    _vertices[vertex_num] = face->node_ptr(vertex_num);
}
