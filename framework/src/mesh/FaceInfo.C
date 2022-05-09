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

FaceInfo::FaceInfo(const ElemInfo * elem_info, unsigned int side, const ElemInfo * neighbor_info)
  : _elem_info(elem_info),
    _neighbor_info(neighbor_info),
    _processor_id(_elem_info->elem()->processor_id()),
    _id(std::make_pair(_elem_info->elem()->id(), side)),

    _elem_side_id(side),
    _face(const_cast<Elem *>(_elem_info->elem())->build_side_ptr(_elem_side_id)),
    _face_area(_face->volume()),
    _face_centroid(_face->vertex_average()),
    // the neighbor info does not exist for domain boundaries. Additionally, we don't have any info
    // if the neighbor is a RemoteElem. This can happen for ghosted elements on the edge of a
    // stencil, for whom we have may have deleted some of their neighbors when running with a
    // distributed mesh
    _valid_neighbor(_neighbor_info->elem() && _neighbor_info->elem() != remote_elem),
    _neighbor_side_id(_valid_neighbor
                          ? _neighbor_info->elem()->which_neighbor_am_i(_elem_info->elem())
                          : std::numeric_limits<unsigned int>::max()),
    _d_cf(_neighbor_info->centroid() - _elem_info->centroid()),
    _d_cf_mag(_d_cf.norm()),
    _e_cf(_d_cf / _d_cf_mag)
{
  // compute an centroid face normal by using 1-point quadrature
  unsigned int dim = elem_info->elem()->dim();
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
