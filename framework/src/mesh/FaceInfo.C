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

FaceInfo::FaceInfo(const Elem * elem, unsigned int side, const Elem * neighbor)
  : _processor_id(elem->processor_id()),
    _elem(elem),
    _neighbor(neighbor),
    _elem_subdomain_id(elem->subdomain_id()),
    _elem_side_id(side),
    _elem_centroid(elem->centroid()),
    _elem_volume(elem->volume()),
    _face(const_cast<Elem *>(elem)->build_side_ptr(_elem_side_id)),
    _face_area(_face->volume()),
    _face_centroid(_face->centroid()),
    // the neighbor info does not exist for domain boundaries. Additionally, we don't have any info
    // if the neighbor is a RemoteElem. This can happen for ghosted elements on the edge of a
    // stencil, for whom we have may have deleted some of their neighbors when running with a
    // distributed mesh
    _valid_neighbor(neighbor && neighbor != remote_elem),
    _neighbor_subdomain_id(_valid_neighbor ? neighbor->subdomain_id() : Moose::INVALID_BLOCK_ID),
    _neighbor_side_id(_valid_neighbor ? neighbor->which_neighbor_am_i(elem)
                                      : std::numeric_limits<unsigned int>::max()),
    _neighbor_centroid(_valid_neighbor ? neighbor->centroid()
                                       : 2 * (_face_centroid - _elem_centroid) + _elem_centroid),
    _neighbor_volume(_valid_neighbor ? neighbor->volume() : _elem_volume),
    _gc((_neighbor_centroid - _face_centroid).norm() /
        ((_neighbor_centroid - _face_centroid).norm() + (_elem_centroid - _face_centroid).norm())),
    _d_cf(_neighbor_centroid - _elem_centroid),
    _d_cf_mag(_d_cf.norm()),
    _e_cf(_d_cf / _d_cf_mag)
{
  // compute an centroid face normal by using 1-point quadrature
  unsigned int dim = elem->dim();
  std::unique_ptr<FEBase> fe(FEBase::build(dim, FEType(elem->default_order())));
  QGauss qface(dim - 1, CONSTANT);
  fe->attach_quadrature_rule(&qface);
  const std::vector<Point> & normals = fe->get_normals();
  fe->reinit(elem, _elem_side_id);
  mooseAssert(normals.size() == 1, "FaceInfo construction broken w.r.t. computing face normals");
  _normal = normals[0];

  _vertices.resize(_face->n_vertices());
  for (const auto vertex_num : make_range(_face->n_vertices()))
    _vertices[vertex_num] = _face->node_ptr(vertex_num);

  // find other_neighbors with the potential other neighors that would be present at the
  // intersection of a lowerdim
  _other_neighbors.clear();
  if (_neighbor &&
      (_neighbor_side_id == std::numeric_limits<unsigned int>::max() ||
       elem->which_neighbor_am_i(neighbor) == std::numeric_limits<unsigned int>::max()) &&
      elem->dim() == neighbor->dim())
  {
    unsigned int side_neighbor = 0;
    const Elem * elem_loop;
    const Elem * neighbor_loop = elem->neighbor_ptr(side);
    while (neighbor_loop->id() != elem->id()) // condition to continue
    {
      if (neighbor_loop->id() != neighbor->id())
        _other_neighbors.push_back(neighbor_loop);
      elem_loop = neighbor_loop;
      side_neighbor = 0;
      while (side_neighbor < elem_loop->n_sides())
      {
        std::unique_ptr<const Elem> face_i(elem_loop->build_side_ptr(side_neighbor, false));
        if (face_i->centroid() == _face_centroid)
          break;
        side_neighbor++;
      }
      neighbor_loop = elem_loop->neighbor_ptr(side_neighbor);
    }
  }
}
