//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUTypes.h"

#include "MooseMesh.h"

#include "libmesh/elem_range.h"
#include "libmesh/fe_base.h"
#include "libmesh/fe_type.h"

class FEProblemBase;

namespace Moose
{
namespace Kokkos
{

/**
 * The Kokkos assembly class
 */
class Assembly : public MeshHolder
{
public:
  /**
   * Constructor
   * @param problem The MOOSE problem
   */
  Assembly(FEProblemBase & problem);
  /**
   * Initialize assembly
   */
  void init();

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the FE type ID
   * @param type The libMesh FEType object
   * @returns The FE type ID
   */
  auto getFETypeID(FEType type) const { return libmesh_map_find(_fe_type_map, type); }
  /**
   * Get the maximum number of quadrature points per element across the entire mesh
   * @returns The maximum number of quadrature points per element
   */
  KOKKOS_FUNCTION auto getMaxQpsPerElem() const { return _max_qps_per_elem.last(); }
  /**
   * Get the maximum number of quadrature points per element for a subdomain
   * @param subdomain The subdomain ID
   * @returns The maximum number of quadrature points per element
   */
  KOKKOS_FUNCTION auto getMaxQpsPerElem(SubdomainID subdomain) const
  {
    return _max_qps_per_elem[subdomain];
  }
  /**
   * Get the total number of elemental quadrature points in a subdomain
   * @param subdomain The subdomain ID
   * @returns The number of quadrature points
   */
  KOKKOS_FUNCTION auto getNumQps(SubdomainID subdomain) const
  {
    return _qp_offset[subdomain].last();
  }
  /**
   * Get the number of quadrature points of an element
   * @param info The element information object
   * @returns The number of quadrature points
   */
  KOKKOS_FUNCTION auto getNumQps(const ElementInfo & info) const
  {
    return _n_qps[info.subdomain][info.local_id];
  }
  /**
   * Get the total number of facial quadrature points in a subdomain
   * Note: this number does not represent the real number of facial quadrature points but only
   * counts the facial quadrature points that need global caching, such as face material properties
   * @param subdomain The subdomain ID
   * @returns The number of quadrature points
   */
  KOKKOS_FUNCTION auto getNumFaceQps(SubdomainID subdomain) const
  {
    return _qp_offset_face[subdomain].last();
  }
  /**
   * Get the number of quadrature points of a side of an element
   * @param info The element information object
   * @param side The side index
   * @returns The number of quadrature points
   */
  KOKKOS_FUNCTION auto getNumFaceQps(const ElementInfo & info, unsigned int side) const
  {
    return _n_qps_face[info.subdomain](side, info.local_id);
  }
  /**
   * Get the starting offset of quadrature points of an element into the global quadrature point
   * index
   * @param info The element information object
   * @returns The starting offset
   */
  KOKKOS_FUNCTION auto getQpOffset(const ElementInfo & info) const
  {
    return _qp_offset[info.subdomain][info.local_id];
  }
  /**
   * Get the starting offset of quadrature points of a side of an element into the global quadrature
   * point index
   * @param info The element information object
   * @param side The side index
   * @returns The starting offset
   */
  KOKKOS_FUNCTION auto getQpFaceOffset(const ElementInfo & info, unsigned int side) const
  {
    return _qp_offset_face[info.subdomain](side, info.local_id);
  }
  /**
   * Get the number of DOFs of a FE type for an element type
   * @param elem_type The element type ID
   * @param fe_type The FE type ID
   * @returns The number of DOFs
   */
  KOKKOS_FUNCTION auto getNumDofs(unsigned int elem_type, unsigned int fe_type) const
  {
    return _n_dofs(elem_type, fe_type);
  }
  /**
   * Get the shape functions of a FE type for an element type and subdomain
   * @param subdomain The subdomain ID
   * @param elem_type The element type ID
   * @param fe_type The FE type ID
   * @returns The shape functions at quadrature points
   */
  KOKKOS_FUNCTION const auto &
  getPhi(SubdomainID subdomain, unsigned int elem_type, unsigned int fe_type) const
  {
    return _phi(subdomain, elem_type, fe_type);
  }
  /**
   * Get the face shape functions of a FE type for an element type and subdomain
   * @param subdomain The subdomain ID
   * @param elem_type The element type ID
   * @param fe_type The FE type ID
   * @returns The shape functions of all sides at quadrature points
   */
  KOKKOS_FUNCTION const auto &
  getPhiFace(SubdomainID subdomain, unsigned int elem_type, unsigned int fe_type) const
  {
    return _phi_face(subdomain, elem_type, fe_type);
  }
  /**
   * Get the gradient of shape functions of a FE type for an element type and subdomain
   * @param subdomain The subdomain ID
   * @param elem_type The element type ID
   * @param fe_type The FE type ID
   * @returns The gradient of shape functions at quadrature points
   */
  KOKKOS_FUNCTION const auto &
  getGradPhi(SubdomainID subdomain, unsigned int elem_type, unsigned int fe_type) const
  {
    return _grad_phi(subdomain, elem_type, fe_type);
  }
  /**
   * Get the gradient of face shape functions of a FE type for an element type and subdomain
   * @param subdomain The subdomain ID
   * @param elem_type The element type ID
   * @param fe_type The FE type ID
   * @returns The gradient of shape functions of all sides at quadrature points
   */
  KOKKOS_FUNCTION const auto &
  getGradPhiFace(SubdomainID subdomain, unsigned int elem_type, unsigned int fe_type) const
  {
    return _grad_phi_face(subdomain, elem_type, fe_type);
  }

  /**
   * Get the coordinate transform factor for a point in a subdomain
   * @param subdomain The subdomain ID
   * @param point The point coordinate
   * @returns The coordinate transform factor
   */
  KOKKOS_FUNCTION inline Real coordTransformFactor(SubdomainID subdomain, Real3 point) const;
  /**
   * Compute physical transformation data for an element
   * @param info The element information object
   * @param qp The local quadrature point index
   * @param jacobian The pointer to store the inverse of Jacobian matrix
   * @param JxW The pointer to store transformed Jacobian weight
   * @param q_points The pointer to store physical quadrature point coordinate
   */
  KOKKOS_FUNCTION inline void computePhysicalMap(const ElementInfo & info,
                                                 unsigned int qp,
                                                 Real33 * jacobian,
                                                 Real * JxW,
                                                 Real3 * q_points) const;
  /**
   * Compute physical transformation data for a side
   * @param info The element information object
   * @param side The side index
   * @param qp The local quadrature point index
   * @param jacobian The pointer to store the inverse of Jacobian matrix
   * @param JxW The pointer to store transformed Jacobian weight
   * @param q_points The pointer to store physical quadrature point coordinate
   */
  KOKKOS_FUNCTION inline void computePhysicalMap(const ElementInfo & info,
                                                 unsigned int side,
                                                 unsigned int qp,
                                                 Real33 * jacobian,
                                                 Real * JxW,
                                                 Real3 * q_points) const;

  /**
   * Get the list of boundaries to cache face material properties
   * @returns The list of boundaries
   */
  const auto & getMaterialBoundaries() const { return _material_boundaries; }
#endif

private:
  /**
   * Initialize quadrature data
   */
  void initQuadrature();
  /**
   * Initialize shape data
   */
  void initShape();

  /**
   * Reference of the MOOSE problem
   */
  FEProblemBase & _problem;
  /**
   * Reference of the MOOSE mesh
   */
  const MooseMesh & _mesh;
  /**
   * FE type ID map
   */
  std::map<FEType, unsigned int> _fe_type_map;

  /**
   * Mesh dimension
   */
  const unsigned int _dimension;
  /**
   * Coordinate system type of each subdomain
   */
  Array<Moose::CoordinateSystemType> _coord_type;
  /**
   * Radial coordinate index in cylindrical coordinate system
   */
  unsigned int _rz_radial_coord = libMesh::invalid_uint;
  /**
   * General axisymmetric axis of each subdomain in cylindrical coordinate system
   */
  Array<Pair<Real3, Real3>> _rz_axis;

  /**
   * Starting offset into the global quadrature point index
   */
  ///@{
  Array<Array<dof_id_type>> _qp_offset;
  Array<Array2D<dof_id_type>> _qp_offset_face;
  ///@}
  /**
   * Number of quadrature points
   */
  ///@{
  Array<Array<unsigned int>> _n_qps;
  Array<Array2D<unsigned int>> _n_qps_face;
  Array<unsigned int> _max_qps_per_elem;
  ///@}
  /**
   * Quadrature points and weights for reference elements
   */
  ///@{
  Array2D<Array<Real3>> _q_points;
  Array2D<Array<Array<Real3>>> _q_points_face;
  Array2D<Array<Real>> _weights;
  Array2D<Array<Array<Real>>> _weights_face;
  ///@}
  /**
   * Shape functions for reference elements
   */
  ///@{
  Array3D<Array2D<Real>> _phi;
  Array3D<Array3D<Real>> _phi_face;
  Array3D<Array2D<Real3>> _grad_phi;
  Array3D<Array3D<Real3>> _grad_phi_face;
  Array2D<unsigned int> _n_dofs;
  ///@}
  /**
   * Shape functions for computing reference-to-physical maps
   */
  ///@{
  Array2D<Array2D<Real>> _map_phi;
  Array2D<Array3D<Real>> _map_phi_face;
  Array2D<Array3D<Real>> _map_psi_face;
  Array2D<Array2D<Real3>> _map_grad_phi;
  Array2D<Array3D<Real3>> _map_grad_phi_face;
  Array2D<Array3D<Real3>> _map_grad_psi_face;
  ///@}

  /**
   * Boundaries to cache face material properties
   */
  std::set<BoundaryID> _material_boundaries;
};

#ifdef MOOSE_KOKKOS_SCOPE
KOKKOS_FUNCTION inline Real
Assembly::coordTransformFactor(SubdomainID subdomain, Real3 point) const
{
  switch (_coord_type[subdomain])
  {
    case Moose::COORD_XYZ:
      return 1;
    case Moose::COORD_RZ:
      if (_rz_radial_coord == libMesh::invalid_uint)
        return 2 * M_PI *
               (point - _rz_axis[subdomain].first).cross_product(_rz_axis[subdomain].second).norm();
      else
        return 2 * M_PI * point(_rz_radial_coord);
    case Moose::COORD_RSPHERICAL:
      return 4 * M_PI * point(0) * point(0);
  }
}

KOKKOS_FUNCTION inline void
Assembly::computePhysicalMap(const ElementInfo & info,
                             unsigned int qp,
                             Real33 * jacobian,
                             Real * JxW,
                             Real3 * q_points) const
{
  auto sid = info.subdomain;
  auto eid = info.id;
  auto elem_type = info.type;

  auto & phi = _map_phi(sid, elem_type);
  auto & grad_phi = _map_grad_phi(sid, elem_type);

  Real33 J;
  Real3 xyz;

  for (unsigned int node = 0; node < info.n_nodes; ++node)
  {
    auto points = kokkosMesh().getNodePoint(kokkosMesh().getNodeID(eid, node));

    if (jacobian || JxW)
      J += grad_phi(node, qp).cartesian_product(points);

    xyz += phi(node, qp) * points;
  }

  if (jacobian)
    *jacobian = J.inverse(_dimension);
  if (JxW)
    *JxW =
        J.determinant(_dimension) * _weights(sid, elem_type)[qp] * coordTransformFactor(sid, xyz);
  if (q_points)
    *q_points = xyz;
}

KOKKOS_FUNCTION inline void
Assembly::computePhysicalMap(const ElementInfo & info,
                             unsigned int side,
                             unsigned int qp,
                             Real33 * jacobian,
                             Real * JxW,
                             Real3 * q_points) const
{
  auto sid = info.subdomain;
  auto eid = info.id;
  auto elem_type = info.type;

  auto & phi = _map_phi_face(sid, elem_type);
  auto & grad_phi = _map_grad_phi_face(sid, elem_type);

  Real33 J;
  Real3 xyz;

  for (unsigned int node = 0; node < info.n_nodes; ++node)
  {
    auto points = kokkosMesh().getNodePoint(kokkosMesh().getNodeID(eid, node));

    if (jacobian)
      J += grad_phi(node, qp, side).cartesian_product(points);

    if (JxW || q_points)
      xyz += phi(node, qp, side) * points;
  }

  if (jacobian)
    *jacobian = J.inverse(_dimension);
  if (q_points)
    *q_points = xyz;

  if (JxW)
  {
    Real33 J;

    auto & psi = _map_psi_face(sid, elem_type);
    auto & grad_psi = _map_grad_psi_face(sid, elem_type);

    for (unsigned int node = 0; node < info.n_nodes_side[side]; ++node)
    {
      auto points = kokkosMesh().getNodePoint(kokkosMesh().getNodeID(eid, side, node));

      J += grad_psi(node, qp, side).cartesian_product(points);
    }

    *JxW = ::Kokkos::sqrt((J * J.transpose()).determinant(_dimension - 1)) *
           _weights_face(sid, elem_type)[side][qp] * coordTransformFactor(sid, xyz);
  }
}
#endif

/**
 * The Kokkos interface that holds the host reference of the Kokkos assembly and copies it to device
 * during parallel dispatch.
 * Maintains synchronization between host and device Kokkos assemblies and provides access to the
 * appropriate Kokkos assembly depending on the architecture.
 */
class AssemblyHolder
{
public:
  /**
   * Constructor
   * @param assembly The Kokkos assembly
   */
  AssemblyHolder(const Assembly & assembly) : _assembly_host(assembly), _assembly_device(assembly)
  {
  }
  /**
   * Copy constructor
   */
  AssemblyHolder(const AssemblyHolder & holder)
    : _assembly_host(holder._assembly_host), _assembly_device(holder._assembly_host)
  {
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the const reference of the Kokkos assembly
   * @returns The const reference of the Kokkos assembly depending on the architecture this function
   * is being called on
   */
  KOKKOS_FUNCTION const Assembly & kokkosAssembly() const
  {
    KOKKOS_IF_ON_HOST(return _assembly_host;)
    KOKKOS_IF_ON_DEVICE(return _assembly_device;)
  }
#endif

private:
  /**
   * Host reference of the Kokkos assembly
   */
  const Assembly & _assembly_host;
  /**
   * Device copy of the Kokkos assembly
   */
  const Assembly _assembly_device;
};

} // namespace Kokkos
} // namespace Moose
