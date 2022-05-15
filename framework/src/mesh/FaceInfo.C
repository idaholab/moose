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
    _id(std::make_pair(elem->id(), side)),
    _elem(elem),
    _neighbor(neighbor),
    _elem_subdomain_id(elem->subdomain_id()),
    _elem_side_id(side),
    _elem_centroid(elem->vertex_average()),
    _elem_volume(elem->volume()),
    _face(const_cast<Elem *>(elem)->build_side_ptr(_elem_side_id)),
    _face_area(_face->volume()),
    _face_centroid(_face->vertex_average()),
    // the neighbor info does not exist for domain boundaries. Additionally, we don't have any info
    // if the neighbor is a RemoteElem. This can happen for ghosted elements on the edge of a
    // stencil, for whom we have may have deleted some of their neighbors when running with a
    // distributed mesh
    _valid_neighbor(neighbor && neighbor != remote_elem),
    _neighbor_subdomain_id(_valid_neighbor ? neighbor->subdomain_id() : Moose::INVALID_BLOCK_ID),
    _neighbor_side_id(_valid_neighbor ? neighbor->which_neighbor_am_i(elem)
                                      : std::numeric_limits<unsigned int>::max()),
    _neighbor_centroid(_valid_neighbor ? neighbor->vertex_average()
                                       : 2 * (_face_centroid - _elem_centroid) + _elem_centroid),
    _neighbor_volume(_valid_neighbor ? neighbor->volume() : _elem_volume),
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

  // Compute the position of the intersection of e_CF and the surface
  _r_intersection =
      _elem_centroid + (((_face_centroid - _elem_centroid) * _normal) / (_e_cf * _normal)) * _e_cf;

  // For interpolation coefficients
  _gc = (_neighbor_centroid - _r_intersection).norm() / _d_cf_mag;
}
