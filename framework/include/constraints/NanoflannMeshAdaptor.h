//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// For MooseIndex
#include "MooseTypes.h"

#include "libmesh/libmesh_config.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/mesh_base.h"
#include "libmesh/point.h"
#include "libmesh/elem.h"
#ifdef LIBMESH_HAVE_NANOFLANN
#include "libmesh/nanoflann.hpp"
#else
SORRY THIS APPLICATION REQUIRES NANOFLANN
#endif

// Using statements. These are used so that the code below does not
// need to be littered with libMesh:: disambiguations.
using libMesh::Point;
using libMesh::MeshBase;
using libMesh::Real;
using libMesh::subdomain_id_type;
using libMesh::dof_id_type;
using libMesh::Elem;

/**
 * This allows us to adapt the MeshBase class for use with nanoflann.
 *
 * Taken from detect_slit.cc
 */
template <unsigned int Dim>
class NanoflannMeshAdaptor
{
private:
  // Constant reference to the Mesh we are adapting for use in Nanoflann
  const MeshBase & _mesh;

public:
  NanoflannMeshAdaptor(const MeshBase & mesh) : _mesh(mesh) {}

  /**
   * libMesh \p Point coordinate type
   */
  typedef Real coord_t;

  /**
   * Must return the number of data points
   */
  inline size_t kdtree_get_point_count() const { return _mesh.n_nodes(); }

  /**
   * Returns the distance between the vector "p1[0:size-1]"
   * and the data point with index "idx_p2" stored in _mesh
   */
  inline coord_t kdtree_distance(const coord_t * p1, const size_t idx_p2, size_t size) const
  {
    libmesh_assert_equal_to(size, Dim);

    // Construct a libmesh Point object from the input coord_t.  This
    // assumes LIBMESH_DIM==3.
    Point point1(p1[0], size > 1 ? p1[1] : 0., size > 2 ? p1[2] : 0.);

    // Get the referred-to point from the Mesh
    const Point & point2 = _mesh.point(idx_p2);

    // Compute Euclidean distance, squared
    return (point1 - point2).norm_sq();
  }

  /**
   * Returns the dim'th component of the idx'th point in the class.
   */
  inline coord_t kdtree_get_pt(const size_t idx, int dim) const
  {
    libmesh_assert_less(dim, (int)Dim);
    libmesh_assert_less(idx, _mesh.n_nodes());
    libmesh_assert_less(dim, 3);

    return _mesh.point(idx)(dim);
  }

  /**
   * Optional bounding-box computation: return false to default to a
   * standard bbox computation loop.
   */
  template <class BBOX>
  bool kdtree_get_bbox(BBOX & /* bb */) const
  {
    return false;
  }
};

// Useful typedefs for working with NanoflannMeshAdaptors.

// Declare a type templated on NanoflannMeshAdaptor
typedef nanoflann::L2_Simple_Adaptor<Real, NanoflannMeshAdaptor<3>> adatper_t;

// Declare a KDTree type based on NanoflannMeshAdaptor
typedef nanoflann::KDTreeSingleIndexAdaptor<adatper_t, NanoflannMeshAdaptor<3>, 3> kd_tree_t;

/**
 * Special adaptor that works with subdomains of the Mesh.  When
 * nanoflann asks for the distance between points p1 and p2, and p2 is
 * not a Point of an element in the required subdomain, a "large"
 * distance is returned. Likewise, when the coordinates of a point
 * not in the subdomain are requested, a point at "infinity" is returned.
 */
template <unsigned int Dim>
class NanoflannMeshSubdomainAdaptor
{
private:
  // Constant reference to the Mesh we are adapting for use in Nanoflann
  const MeshBase & _mesh;

  // This could be generalized to a std::set of subodmain ids.
  subdomain_id_type _sid;

  // Indices of points that are attached to elements in the requested subdomain.
  std::set<dof_id_type> _legal_point_indices;

public:
  NanoflannMeshSubdomainAdaptor(const MeshBase & mesh, subdomain_id_type s) : _mesh(mesh), _sid(s)
  {
    // Loop over the elements of the Mesh, for those in the requested
    // subdomain, add its node ids to the _legal_point_indices set.
    for (const auto & elem : _mesh.active_element_ptr_range())
      if (elem->subdomain_id() == _sid)
        for (MooseIndex(elem->n_vertices()) n = 0; n < elem->n_vertices(); ++n)
          _legal_point_indices.insert(elem->node_id(n));
  }

  /**
   * libMesh \p Point coordinate type
   */
  typedef Real coord_t;

  /**
   * Must return the number of data points
   */
  inline size_t kdtree_get_point_count() const { return _mesh.n_nodes(); }

  /**
   * Returns the distance between the vector "p1[0:size-1]"
   * and the data point with index "idx_p2" stored in _mesh
   */
  inline coord_t kdtree_distance(const coord_t * p1, const size_t idx_p2, size_t size) const
  {
    libmesh_assert_equal_to(size, Dim);

    // If this is not a valid point, then return a "large" distance.
    if (!_legal_point_indices.count(static_cast<dof_id_type>(idx_p2)))
      return std::numeric_limits<coord_t>::max();

    // Construct a libmesh Point object from the input coord_t.  This
    // assumes LIBMESH_DIM==3.
    Point point1(p1[0], size > 1 ? p1[1] : 0., size > 2 ? p1[2] : 0.);

    // Get the referred-to point from the Mesh
    const Point & point2 = _mesh.point(idx_p2);

    // Compute Euclidean distance, squared
    return (point1 - point2).norm_sq();
  }

  /**
   * Returns the dim'th component of the idx'th point in the class.
   */
  inline coord_t kdtree_get_pt(const size_t idx, int dim) const
  {
    libmesh_assert_less(dim, (int)Dim);
    libmesh_assert_less(idx, _mesh.n_nodes());
    libmesh_assert_less(dim, 3);

    // If this is not a valid point, then return a "large" distance.
    if (!_legal_point_indices.count(static_cast<dof_id_type>(idx)))
      return std::numeric_limits<coord_t>::max();

    return _mesh.point(idx)(dim);
  }

  /**
   * Optional bounding-box computation: return false to default to a
   * standard bbox computation loop.
   */
  template <class BBOX>
  bool kdtree_get_bbox(BBOX & /* bb */) const
  {
    return false;
  }
};

// Useful typedefs for working with NanoflannMeshAdaptors.

// Declare a type templated on NanoflannMeshAdaptor
typedef nanoflann::L2_Simple_Adaptor<Real, NanoflannMeshSubdomainAdaptor<3>> subdomain_adatper_t;

// Declare a KDTree type based on NanoflannMeshAdaptor
typedef nanoflann::
    KDTreeSingleIndexAdaptor<subdomain_adatper_t, NanoflannMeshSubdomainAdaptor<3>, 3>
        subdomain_kd_tree_t;
