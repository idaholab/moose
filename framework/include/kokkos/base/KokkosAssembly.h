//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"

#include "MooseMesh.h"

#include "libmesh/elem_range.h"
#include "libmesh/fe_base.h"
#include "libmesh/fe_type.h"

class FEProblemBase;

namespace Moose::Kokkos
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
  unsigned int getFETypeID(FEType type) const { return libmesh_map_find(_fe_type_map, type); }
  /**
   * Get the mesh dimension
   * @returns The mesh dimension
   */
  KOKKOS_FUNCTION unsigned int getDimension() const { return _dimension; }
  /**
   * Get the maximum number of quadrature points per element in the current partition
   * @returns The maximum number of quadrature points per element
   */
  KOKKOS_FUNCTION unsigned int getMaxQpsPerElem() const { return _max_qps_per_elem; }
  /**
   * Get the total number of elemental quadrature points in a subdomain
   * @param subdomain The contiguous subdomain ID
   * @returns The number of quadrature points
   */
  KOKKOS_FUNCTION dof_id_type getNumQps(ContiguousSubdomainID subdomain) const
  {
    return _n_subdomain_qps[subdomain];
  }
  /**
   * Get the number of quadrature points of an element
   * @param info The element information object
   * @returns The number of quadrature points
   */
  KOKKOS_FUNCTION unsigned int getNumQps(ElementInfo info) const { return _n_qps[info.id]; }
  /**
   * Get the total number of facial quadrature points in a subdomain
   * NOTE: This number does not represent the real number of facial quadrature points but only
   * the facial quadrature points that need global caching, such as face material properties
   * @param subdomain The contiguous subdomain ID
   * @returns The number of quadrature points
   */
  KOKKOS_FUNCTION dof_id_type getNumFaceQps(ContiguousSubdomainID subdomain) const
  {
    return _n_subdomain_qps_face[subdomain];
  }
  /**
   * Get the number of quadrature points of a side of an element
   * @param info The element information object
   * @param side The side index
   * @returns The number of quadrature points
   */
  KOKKOS_FUNCTION unsigned int getNumFaceQps(ElementInfo info, unsigned int side) const
  {
    return _n_qps_face(side, info.id);
  }
  /**
   * Get the starting offset of quadrature points of an element into the global quadrature point
   * index
   * @param info The element information object
   * @returns The starting offset
   */
  KOKKOS_FUNCTION dof_id_type getQpOffset(ElementInfo info) const { return _qp_offset[info.id]; }
  /**
   * Get the starting offset of quadrature points of a side of an element into the global quadrature
   * point index
   * @param info The element information object
   * @param side The side index
   * @returns The starting offset
   */
  KOKKOS_FUNCTION dof_id_type getQpFaceOffset(ElementInfo info, unsigned int side) const
  {
    return _qp_offset_face(side, info.id);
  }
  /**
   * Get the index of a side of an element into the element-constant face material property data
   * @param info The element information object
   * @param side The side index
   * @returns The index
   */
  KOKKOS_FUNCTION dof_id_type getElemFacePropertyIndex(ElementInfo info, unsigned int side) const
  {
    return _elem_face_property_idx(side, info.id);
  }
  /**
   * Get the size of element-constant face material property data storage of a subdomain
   * @param subdomain The contiguous subdomain ID
   * @returns The storage size
   */
  KOKKOS_FUNCTION dof_id_type getElemFacePropertySize(ContiguousSubdomainID subdomain) const
  {
    return _n_elem_face_properties[subdomain];
  }
  /**
   * Get the number of DOFs of a FE type for an element type
   * @param elem_type The element type ID
   * @param fe_type The FE type ID
   * @returns The number of DOFs
   */
  KOKKOS_FUNCTION unsigned int getNumDofs(unsigned int elem_type, unsigned int fe_type) const
  {
    return _n_dofs(elem_type, fe_type);
  }
  /**
   * Get the shape functions of a FE type for an element type and subdomain
   * @param subdomain The contiguous subdomain ID
   * @param elem_type The element type ID
   * @param fe_type The FE type ID
   * @returns The shape functions at quadrature points
   */
  KOKKOS_FUNCTION const auto &
  getPhi(ContiguousSubdomainID subdomain, unsigned int elem_type, unsigned int fe_type) const
  {
    return _phi(subdomain, elem_type, fe_type);
  }
  /**
   * Get the face shape functions of a FE type for an element type and subdomain
   * @param subdomain The contiguous subdomain ID
   * @param elem_type The element type ID
   * @param fe_type The FE type ID
   * @returns The shape functions of all sides at quadrature points
   */
  KOKKOS_FUNCTION const auto &
  getPhiFace(ContiguousSubdomainID subdomain, unsigned int elem_type, unsigned int fe_type) const
  {
    return _phi_face(subdomain, elem_type, fe_type);
  }
  /**
   * Get the gradient of shape functions of a FE type for an element type and subdomain
   * @param subdomain The contiguous subdomain ID
   * @param elem_type The element type ID
   * @param fe_type The FE type ID
   * @returns The gradient of shape functions at quadrature points
   */
  KOKKOS_FUNCTION const auto &
  getGradPhi(ContiguousSubdomainID subdomain, unsigned int elem_type, unsigned int fe_type) const
  {
    return _grad_phi(subdomain, elem_type, fe_type);
  }
  /**
   * Get the gradient of face shape functions of a FE type for an element type and subdomain
   * @param subdomain The contiguous subdomain ID
   * @param elem_type The element type ID
   * @param fe_type The FE type ID
   * @returns The gradient of shape functions of all sides at quadrature points
   */
  KOKKOS_FUNCTION const auto & getGradPhiFace(ContiguousSubdomainID subdomain,
                                              unsigned int elem_type,
                                              unsigned int fe_type) const
  {
    return _grad_phi_face(subdomain, elem_type, fe_type);
  }
  /**
   * Get the inverse of Jacobian matrix of an element quadrature point
   * @param info The element information object
   * @param qp The local quadrature point index
   * @returns The inverse of Jacobian matrix
   */
  KOKKOS_FUNCTION Real33 getJacobian(ElementInfo info, unsigned int qp) const
  {
    return _jacobian[info.subdomain][getQpOffset(info) + qp];
  }
  /**
   * Get the transformed Jacobian weight of an element quadrature point
   * @param info The element information object
   * @param qp The local quadrature point index
   * @returns The inverse of Jacobian matrix
   */
  KOKKOS_FUNCTION Real getJxW(ElementInfo info, unsigned int qp) const
  {
    return _jxw[info.subdomain][getQpOffset(info) + qp];
  }
  /**
   * Get the coordinate of an element quadrature point
   * @param info The element information object
   * @param qp The local quadrature point index
   * @returns The inverse of Jacobian matrix
   */
  KOKKOS_FUNCTION Real3 getQPoint(ElementInfo info, unsigned int qp) const
  {
    return _xyz[info.subdomain][getQpOffset(info) + qp];
  }

  /**
   * Get the coordinate transform factor for a point in a subdomain
   * @param subdomain The contiguous subdomain ID
   * @param point The point coordinate
   * @returns The coordinate transform factor
   */
  KOKKOS_FUNCTION Real coordTransformFactor(const ContiguousSubdomainID subdomain,
                                            const Real3 point) const;
  /**
   * Compute physical transformation data for an element
   * @param info The element information object
   * @param qp The local quadrature point index
   * @param jacobian The pointer to store the inverse of Jacobian matrix
   * @param JxW The pointer to store transformed Jacobian weight
   * @param q_points The pointer to store physical quadrature point coordinate
   */
  KOKKOS_FUNCTION void computePhysicalMap(const ElementInfo info,
                                          const unsigned int qp,
                                          Real33 * const jacobian,
                                          Real * const JxW,
                                          Real3 * const q_points) const;
  /**
   * Compute physical transformation data for a side
   * @param info The element information object
   * @param side The side index
   * @param qp The local quadrature point index
   * @param jacobian The pointer to store the inverse of Jacobian matrix
   * @param JxW The pointer to store transformed Jacobian weight
   * @param q_points The pointer to store physical quadrature point coordinate
   */
  KOKKOS_FUNCTION void computePhysicalMap(const ElementInfo info,
                                          const unsigned int side,
                                          const unsigned int qp,
                                          Real33 * const jacobian,
                                          Real * const JxW,
                                          Real3 * const q_points) const;

  /**
   * Kokkos function for caching physical maps on element quadrature points
   */
  KOKKOS_FUNCTION void operator()(const ThreadID tid) const;

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
   * Cache physical maps on element quadrature points
   */
  void cachePhysicalMap();

  /**
   * Reference of the MOOSE problem
   */
  FEProblemBase & _problem;
  /**
   * Reference of the MOOSE mesh
   */
  MooseMesh & _mesh;
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
   * NOTE: The global quadrature point index is subdomain-wise
   */
  ///@{
  Array<dof_id_type> _qp_offset;
  Array2D<dof_id_type> _qp_offset_face;
  ///@}
  /**
   * Number of quadrature points
   */
  ///@{
  Array<unsigned int> _n_qps;
  Array2D<unsigned int> _n_qps_face;

  unsigned int _max_qps_per_elem = 0;

  Array<dof_id_type> _n_subdomain_qps;
  Array<dof_id_type> _n_subdomain_qps_face;
  ///@}
  /**
   * Index into the element-constant face material property data
   */
  ///@{
  Array2D<dof_id_type> _elem_face_property_idx;
  Array<dof_id_type> _n_elem_face_properties;
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
  Array3D<Array<Array2D<Real>>> _phi_face;
  Array3D<Array2D<Real3>> _grad_phi;
  Array3D<Array<Array2D<Real3>>> _grad_phi_face;
  Array2D<unsigned int> _n_dofs;
  ///@}
  /**
   * Shape functions for computing reference-to-physical maps
   */
  ///@{
  Array2D<Array2D<Real>> _map_phi;
  Array2D<Array<Array2D<Real>>> _map_phi_face;
  Array2D<Array<Array2D<Real>>> _map_psi_face;
  Array2D<Array2D<Real3>> _map_grad_phi;
  Array2D<Array<Array2D<Real3>>> _map_grad_phi_face;
  Array2D<Array<Array2D<Real3>>> _map_grad_psi_face;
  ///@}
  /**
   * Cached physical maps on element quadrature points
   */
  ///@{
  Array<Array<Real33>> _jacobian;
  Array<Array<Real>> _jxw;
  Array<Array<Real3>> _xyz;
  ///@}

  /**
   * Boundaries to cache face material properties
   */
  std::set<BoundaryID> _material_boundaries;
};

#ifdef MOOSE_KOKKOS_SCOPE
KOKKOS_FUNCTION inline Real
Assembly::coordTransformFactor(const ContiguousSubdomainID subdomain, const Real3 point) const
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
    default:
      return 0;
  }
}

KOKKOS_FUNCTION inline void
Assembly::computePhysicalMap(const ElementInfo info,
                             const unsigned int qp,
                             Real33 * const jacobian,
                             Real * const JxW,
                             Real3 * const q_points) const
{
  auto sid = info.subdomain;
  auto eid = info.id;
  auto elem_type = info.type;
  auto num_nodes = kokkosMesh().getNumNodes(elem_type);

  auto & phi = _map_phi(sid, elem_type);
  auto & grad_phi = _map_grad_phi(sid, elem_type);

  Real33 J;
  Real3 xyz;

  for (unsigned int node = 0; node < num_nodes; ++node)
  {
    auto points = kokkosMesh().getNodePoint(kokkosMesh().getContiguousNodeID(eid, node));

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
Assembly::computePhysicalMap(const ElementInfo info,
                             const unsigned int side,
                             const unsigned int qp,
                             Real33 * const jacobian,
                             Real * const JxW,
                             Real3 * const q_points) const
{
  auto sid = info.subdomain;
  auto eid = info.id;
  auto elem_type = info.type;
  auto num_nodes = kokkosMesh().getNumNodes(elem_type);
  auto num_side_nodes = kokkosMesh().getNumNodes(elem_type, side);

  auto & phi = _map_phi_face(sid, elem_type)(side);
  auto & grad_phi = _map_grad_phi_face(sid, elem_type)(side);

  Real33 J;
  Real3 xyz;

  for (unsigned int node = 0; node < num_nodes; ++node)
  {
    auto points = kokkosMesh().getNodePoint(kokkosMesh().getContiguousNodeID(eid, node));

    if (jacobian)
      J += grad_phi(node, qp).cartesian_product(points);

    if (JxW || q_points)
      xyz += phi(node, qp) * points;
  }

  if (jacobian)
    *jacobian = J.inverse(_dimension);
  if (q_points)
    *q_points = xyz;

  if (JxW)
  {
    Real33 J;

    auto & grad_psi = _map_grad_psi_face(sid, elem_type)(side);

    for (unsigned int node = 0; node < num_side_nodes; ++node)
    {
      auto points = kokkosMesh().getNodePoint(kokkosMesh().getContiguousNodeID(eid, side, node));

      J += grad_psi(node, qp).cartesian_product(points);
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

    return _assembly_device;
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

} // namespace Moose::Kokkos
