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
class Assembly;

class GPUAssembly : public GPUMeshHolder
{
public:
  // Constructor
  GPUAssembly(FEProblemBase & problem);
  // Initialize assembly
  void init();
  void initQuadrature();
  void initShape();

#ifdef MOOSE_GPU_SCOPE
  // Get the FE type number
  auto getFETypeNum(FEType type) const { return _fe_type_map.at(type); }
  // Get the number of quadrature points
  KOKKOS_FUNCTION auto getMaxQpsPerElem() const { return _max_qps_per_elem.last(); }
  KOKKOS_FUNCTION auto getMaxQpsPerElem(SubdomainID subdomain) const
  {
    return _max_qps_per_elem[subdomain];
  }
  KOKKOS_FUNCTION auto getNumQps(SubdomainID subdomain) const
  {
    return _qp_offset[subdomain].last();
  }
  KOKKOS_FUNCTION auto getNumQps(GPUElementInfo info) const
  {
    return _n_qps[info.subdomain][info.local_id];
  }
  KOKKOS_FUNCTION auto getNumFaceQps(SubdomainID subdomain) const
  {
    return _qp_offset_face[subdomain].last();
  }
  KOKKOS_FUNCTION auto getNumFaceQps(GPUElementInfo info, unsigned int side) const
  {
    return _n_qps_face[info.subdomain](side, info.local_id);
  }
  // Get the quadrature point offset
  KOKKOS_FUNCTION auto getQpOffset(GPUElementInfo info) const
  {
    return _qp_offset[info.subdomain][info.local_id];
  }
  KOKKOS_FUNCTION auto getQpFaceOffset(GPUElementInfo info, unsigned int side) const
  {
    return _qp_offset_face[info.subdomain](side, info.local_id);
  }
  // Get the shape data
  KOKKOS_FUNCTION auto getNumDofs(unsigned int elem_type, unsigned int fe_type) const
  {
    return _n_dofs(elem_type, fe_type);
  }
  KOKKOS_FUNCTION const auto &
  getPhi(SubdomainID subdomain, unsigned int elem_type, unsigned int fe_type) const
  {
    return _phi(subdomain, elem_type, fe_type);
  }
  KOKKOS_FUNCTION const auto &
  getPhiFace(SubdomainID subdomain, unsigned int elem_type, unsigned int fe_type) const
  {
    return _phi_face(subdomain, elem_type, fe_type);
  }
  KOKKOS_FUNCTION const auto &
  getGradPhi(SubdomainID subdomain, unsigned int elem_type, unsigned int fe_type) const
  {
    return _grad_phi(subdomain, elem_type, fe_type);
  }
  KOKKOS_FUNCTION const auto &
  getGradPhiFace(SubdomainID subdomain, unsigned int elem_type, unsigned int fe_type) const
  {
    return _grad_phi_face(subdomain, elem_type, fe_type);
  }

  KOKKOS_FUNCTION Real coordTransformFactor(SubdomainID subdomain, Real3 point) const
  {
    switch (_coord_type[subdomain])
    {
      case Moose::COORD_XYZ:
        return 1;
      case Moose::COORD_RZ:
        if (_rz_radial_coord == libMesh::invalid_uint)
          return 2 * M_PI *
                 (point - _rz_axis[subdomain].first)
                     .cross_product(_rz_axis[subdomain].second)
                     .norm();
        else
          return 2 * M_PI * point(_rz_radial_coord);
      case Moose::COORD_RSPHERICAL:
        return 4 * M_PI * point(0) * point(0);
    }
  }
  KOKKOS_FUNCTION void computePhysicalMap(
      GPUElementInfo info, unsigned int qp, Real33 * jacobian, Real * JxW, Real3 * q_points) const
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
      auto points = mesh().getNodePoint(mesh().getNodeID(eid, node));

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
  KOKKOS_FUNCTION void computePhysicalMap(GPUElementInfo info,
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
      auto points = mesh().getNodePoint(mesh().getNodeID(eid, node));

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
        auto points = mesh().getNodePoint(mesh().getNodeID(eid, side, node));

        J += grad_psi(node, qp, side).cartesian_product(points);
      }

      *JxW = std::sqrt((J * J.transpose()).determinant(_dimension - 1)) *
             _weights_face(sid, elem_type)[side][qp] * coordTransformFactor(sid, xyz);
    }
  }

#endif

private:
  // Reference to MOOSE problem
  FEProblemBase & _problem;
  // Reference to MOOSE mesh
  const MooseMesh & _mesh;
  // Unique FE type map
  std::map<FEType, unsigned int> _fe_type_map;

private:
  // Number of subdomains
  unsigned int _n_subdomains = 0;
  // Number of element types
  unsigned int _n_elem_types = 0;
  // Mesh dimension
  unsigned int _dimension = 0;
  // Coordinate system type
  GPUArray<Moose::CoordinateSystemType> _coord_type;
  // Radial coordinate index in cylindrical coordinate system
  unsigned int _rz_radial_coord = libMesh::invalid_uint;
  // General axisymmetric axis in cylindrical coordinate system
  GPUArray<GPUPair<Real3, Real3>> _rz_axis;

private:
  // Quadrature point offset of each element
  GPUArray<GPUArray<dof_id_type>> _qp_offset;
  GPUArray<GPUArray2D<dof_id_type>> _qp_offset_face;
  // Number of quadrature points of each element
  GPUArray<GPUArray<unsigned int>> _n_qps;
  GPUArray<GPUArray2D<unsigned int>> _n_qps_face;
  GPUArray<unsigned int> _max_qps_per_elem;

private:
  // Reference quadrature data
  GPUArray2D<GPUArray<Real3>> _q_points;
  GPUArray2D<GPUArray<GPUArray<Real3>>> _q_points_face;
  GPUArray2D<GPUArray<Real>> _weights;
  GPUArray2D<GPUArray<GPUArray<Real>>> _weights_face;
  // Reference shape data
  GPUArray3D<GPUArray2D<Real>> _phi;
  GPUArray3D<GPUArray3D<Real>> _phi_face;
  GPUArray3D<GPUArray2D<Real3>> _grad_phi;
  GPUArray3D<GPUArray3D<Real3>> _grad_phi_face;
  GPUArray2D<unsigned int> _n_dofs;
  // Reference map data
  GPUArray2D<GPUArray2D<Real>> _map_phi;
  GPUArray2D<GPUArray3D<Real>> _map_phi_face;
  GPUArray2D<GPUArray3D<Real>> _map_psi_face;
  GPUArray2D<GPUArray2D<Real3>> _map_grad_phi;
  GPUArray2D<GPUArray3D<Real3>> _map_grad_phi_face;
  GPUArray2D<GPUArray3D<Real3>> _map_grad_psi_face;
};

class GPUAssemblyHolder
{
private:
  // Copy of GPU assembly
  GPUAssembly _assembly_device;
  // Reference to GPU assembly
  GPUAssembly & _assembly_host;

#ifdef MOOSE_GPU_SCOPE
public:
  KOKKOS_FUNCTION const GPUAssembly & assembly() const
  {
    KOKKOS_IF_ON_HOST(return _assembly_host;)
    KOKKOS_IF_ON_DEVICE(return _assembly_device;)
  }
  KOKKOS_FUNCTION GPUAssembly & assembly()
  {
    KOKKOS_IF_ON_HOST(return _assembly_host;)
    KOKKOS_IF_ON_DEVICE(return _assembly_device;)
  }
#endif

public:
  GPUAssemblyHolder(GPUAssembly & assembly) : _assembly_device(assembly), _assembly_host(assembly)
  {
  }
  GPUAssemblyHolder(const GPUAssemblyHolder & holder)
    : _assembly_device(holder._assembly_host), _assembly_host(holder._assembly_host)
  {
  }
};
