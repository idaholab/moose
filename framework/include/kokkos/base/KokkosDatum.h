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
#include "KokkosAssembly.h"
#include "KokkosFESystem.h"
#include "KokkosVariable.h"

namespace Moose::Kokkos
{

/**
 * Generic datum class containing geometric information related to the mesh. This is a base class
 * for derived datum classes for both finite volume and finite element system assembly
 */
class MeshDatum
{
public:
  KOKKOS_FUNCTION
  MeshDatum(ContiguousElementID elem, const unsigned int side, const Mesh & mesh);

  /**
   * Get the Kokkos mesh
   * @returns The Kokkos mesh
   */
  KOKKOS_FUNCTION const Mesh & mesh() const { return _mesh; }

  /**
   * Get the element information object
   * @returns The element information object
   */
  KOKKOS_FUNCTION const ElementInfo & elem() const { return _elem; }

  /**
   * Get the contiguous element ID
   * @returns The contiguous element ID
   */
  KOKKOS_FUNCTION ContiguousElementID elemID() const { return _elem.id; }

  /**
   * Get whether the current side has a neighbor
   * @returns Whether the current side has a neighbor
   */
  KOKKOS_FUNCTION bool hasNeighbor() const;

  /**
   * Get the neighbor element information object
   * @returns The neighbor element information object. If there is no neighbor or if this datum is
   * not meant to represent face data, then this will point to a default constructed \p ElementInfo
   * whose data members represent invalid state (e.g. hold \p invalid_uint, \p invalid_id like
   * values)
   */
  KOKKOS_FUNCTION const ElementInfo & neighbor() const { return _neighbor; }

  /**
   * Get the contiguous neighbor element ID
   * @returns The contiguous neighbor element ID or \p libMesh::DofObject::invalid_id
   */
  KOKKOS_FUNCTION ContiguousElementID neighborID() const { return _neighbor.id; }

  /**
   * Get the side index
   * @returns The side index
   */
  KOKKOS_FUNCTION unsigned int side() const { return _side; }

  /**
   * Get whether the current datum is on a side
   * @returns Whether the current datum is on a side
   */
  KOKKOS_FUNCTION bool isSide() const { return _side != libMesh::invalid_uint; }

  /**
   * Get the contiguous subdomain ID
   * @returns The contiguous subdomain ID
   */
  KOKKOS_FUNCTION ContiguousSubdomainID subdomain() const { return _elem.subdomain; }

  /**
   * Get the contiguous neighbor subdomain ID
   * @returns The contiguous neighbor subdomain ID
   */
  KOKKOS_FUNCTION ContiguousSubdomainID neighborSubdomain() const { return _neighbor.subdomain; }

protected:
  /**
   * Get the neighbor element information object for an element side
   * @param elem The contiguous element ID
   * @param side The side index
   * @param mesh The Kokkos mesh
   * @returns The neighbor element information object, or a default \p ElementInfo (a default
   *          constructed \p ElementInfo's data members all represent invalid state) when \p elem is
   *          \p libMesh::DofObject::invalid_id, \p side is \p libMesh::invalid_uint, or the Kokkos
   *          mesh has no contiguous neighbor ID for the side. The latter includes exterior
   *          boundary sides with no libMesh neighbor and sides whose libMesh neighbor is
   *          \p libMesh::remote_elem. Off-process neighbors present as ghost elements have
   *          contiguous IDs and return their \p ElementInfo.
   */
  static KOKKOS_FUNCTION ElementInfo neighborInfo(ContiguousElementID elem,
                                                  const unsigned int side,
                                                  const Mesh & mesh);

  /**
   * Reference to the Kokkos mesh
   */
  const Mesh & _mesh;

  /**
   * Current element information object
   */
  const ElementInfo _elem;

  /**
   * Current side index
   */
  const unsigned int _side = libMesh::invalid_uint;

  /**
   * Current neighbor element information object
   */
  const ElementInfo _neighbor;
};

KOKKOS_FUNCTION inline MeshDatum::MeshDatum(ContiguousElementID elem,
                                            const unsigned int side,
                                            const Mesh & mesh)
  : _mesh(mesh),
    _elem(elem != libMesh::DofObject::invalid_id ? _mesh.getElementInfo(elem) : ElementInfo{}),
    _side(side),
    _neighbor(neighborInfo(_elem.id, _side, _mesh))
{
}

KOKKOS_FUNCTION inline bool
MeshDatum::hasNeighbor() const
{
  return _neighbor.id != libMesh::DofObject::invalid_id;
}

KOKKOS_FUNCTION inline ElementInfo
MeshDatum::neighborInfo(ContiguousElementID elem, const unsigned int side, const Mesh & mesh)
{
  if (elem == libMesh::DofObject::invalid_id || side == libMesh::invalid_uint)
    return {};

  const auto neighbor = mesh.getNeighbor(elem, side);
  return neighbor == libMesh::DofObject::invalid_id ? ElementInfo{} : mesh.getElementInfo(neighbor);
}

class FVDatum : public MeshDatum
{
public:
  KOKKOS_FUNCTION
  FVDatum(ContiguousElementID elem, const unsigned int side, const Mesh & mesh);

  KOKKOS_FUNCTION
  Real3 faceCentroid() const { return _mesh.getSideCentroid(_elem.id, _side); }

  KOKKOS_FUNCTION
  Real faceDCNMag() const
  {
    return _mesh.getElementCentroidToNeighborCentroidDistance(_elem.id, _side);
  }

  KOKKOS_FUNCTION
  Real faceDCFMag() const
  {
    return _mesh.getElementCentroidToSideCentroidDistance(_elem.id, _side);
  }

  KOKKOS_FUNCTION
  Real faceArea() const { return _mesh.getSideArea(_elem.id, _side); }

  KOKKOS_FUNCTION
  Real3 elementCentroid() const { return _mesh.getElementCentroid(_elem.id); }

  KOKKOS_FUNCTION
  Real elementVolume() const { return _mesh.getElementVolume(_elem.id); }
};

KOKKOS_FUNCTION inline FVDatum::FVDatum(ContiguousElementID elem,
                                        const unsigned int side,
                                        const Mesh & mesh)
  : MeshDatum(elem, side, mesh)
{
}

/**
 * The Kokkos object that holds thread-private data in the parallel operations of any Kokkos object
 */
class Datum : public MeshDatum
{
public:
  /**
   * Constructor for element and side data
   * @param elem The contiguous element ID of the current thread
   * @param side The side index of the current thread
   * @param assembly The Kokkos assembly
   * @param systems The Kokkos systems
   */
  KOKKOS_FUNCTION
  Datum(const ContiguousElementID elem,
        const unsigned int side,
        const Assembly & assembly,
        const Array<FESystem> & systems);

  /**
   * Constructor for node data
   * @param node The contiguous node ID of the current thread
   * @param assembly The Kokkos assembly
   * @param systems The Kokkos systems
   */
  KOKKOS_FUNCTION
  Datum(const ContiguousNodeID node, const Assembly & assembly, const Array<FESystem> & systems);

  /**
   * Get the Kokkos assembly
   * @returns The Kokkos assembly
   */
  KOKKOS_FUNCTION const Assembly & assembly() const { return _assembly; }

  /**
   * Get the Kokkos system
   * @param sys The system number
   * @returns The Kokkos system
   */
  KOKKOS_FUNCTION const FESystem & system(unsigned int sys) const { return _systems[sys]; }

  /**
   * Get the extra element ID
   * @param index The extra element ID index
   * @returns The extra element ID
   */
  KOKKOS_FUNCTION dof_id_type extraElemID(unsigned int index) const
  {
    return isNodal() ? libMesh::DofObject::invalid_id : _mesh.getExtraElementID(_elem.id, index);
  }

  /**
   * Get the contiguous node ID
   * @returns The contiguous node ID
   */
  KOKKOS_FUNCTION ContiguousNodeID node() const { return _node; }

  /**
   * Get the number of local quadrature points
   * @returns The number of local quadrature points
   */
  KOKKOS_FUNCTION unsigned int n_qps() const { return _n_qps; }

  /**
   * Get the starting offset into the global quadrature point index
   * @returns The starting offset
   */
  KOKKOS_FUNCTION dof_id_type qpOffset() const { return _qp_offset; }

  /**
   * Get the index into the property data storage
   * @param constant_option The property constant option
   * @param qp The local quadrature point index
   * @returns The index
   */
  KOKKOS_FUNCTION dof_id_type propertyIdx(const PropertyConstantOption constant_option,
                                          const unsigned int qp) const;

  /**
   * Get whether the current datum is on a node
   * @returns Whether the current datum is on a node
   */
  KOKKOS_FUNCTION bool isNodal() const { return _node != libMesh::DofObject::invalid_id; }

  /**
   * Get whether the a variable is defined on the current node
   * @param var The variable
   * @returns Whether the variable is defined on the current node
   */
  KOKKOS_FUNCTION bool isNodalDefined(const Variable & var) const;

  /**
   * Get the inverse of Jacobian matrix
   * | dxi/dx deta/dx dzeta/dx |
   * | dxi/dy deta/dy dzeta/dy |
   * | dxi/dz deta/dz dzeta/dz |
   * @param qp The local quadrature point index
   * @returns The Jacobian matrix
   */
  KOKKOS_FUNCTION const Real33 & J(const unsigned int qp);

  /**
   * Get the transformed Jacobian weight
   * @param qp The local quadrature point index
   * @returns The transformed Jacobian weights
   */
  KOKKOS_FUNCTION Real JxW(const unsigned int qp);

  /**
   * Get the physical quadrature point coordinate
   * @param qp The local quadrature point index
   * @returns The physical quadrature point coordinate
   */
  KOKKOS_FUNCTION Real3 q_point(const unsigned int qp);

  /**
   * Get the normal vector on surface
   * @param qp The local quadrature point index
   * @returns The normal vector
   */
  KOKKOS_FUNCTION Real3 normals(const unsigned int qp);

  /**
   * Set local parallelization option
   * @param local_thread_id The current local thread ID
   * @param num_local_threads The number of local threads
   */
  KOKKOS_FUNCTION void set_local_parallel(const unsigned int local_thread_id,
                                          const unsigned int num_local_threads)
  {
    _local_thread_id = local_thread_id;
    _num_local_threads = num_local_threads;
  }
  /**
   * Get the current local thread ID
   * @returns The current local thread ID
   */
  KOKKOS_FUNCTION unsigned int local_thread_id() const { return _local_thread_id; }
  /**
   * Get the number of local threads
   * @returns The number of local threads
   */
  KOKKOS_FUNCTION unsigned int num_local_threads() const { return _num_local_threads; }

protected:
  /**
   * Reference of the Kokkos assembly
   */
  const Assembly & _assembly;

  /**
   * Reference of the Kokkos systems
   */
  const Array<FESystem> & _systems;

  /**
   * Current contiguous node ID
   */
  const ContiguousNodeID _node = libMesh::DofObject::invalid_id;

  /**
   * Number of local quadrature points
   */
  const unsigned int _n_qps = 1;

  /**
   * Starting offset into the global quadrature point index
   */
  const dof_id_type _qp_offset = libMesh::DofObject::invalid_id;

  /**
   * Index for element-constant material properties
   */
  const dof_id_type _elem_property_idx = libMesh::DofObject::invalid_id;

private:
  /**
   * Compute and cache the physical transformation data
   * @param qp The local quadrature point index
   */
  KOKKOS_FUNCTION void reinitTransform(const unsigned int qp);

  /**
   * Cached quadrature point index for checking whether the physical transformation data should be
   * recomputed
   */
  unsigned int _cached_qp = libMesh::invalid_uint;
  /**
   * Cached physical transformation data
   */
  ///@{
  Real33 _J;
  Real _JxW;
  Real3 _xyz;
  Real3 _normal;
  ///@}
  /**
   * Thread ID for local parallelization
   */
  unsigned int _local_thread_id = 0;
  /**
   * Number of threads for local parallelization
   */
  unsigned int _num_local_threads = 1;
};

KOKKOS_FUNCTION inline Datum::Datum(const ContiguousElementID elem,
                                    const unsigned int side,
                                    const Assembly & assembly,
                                    const Array<FESystem> & systems)
  : MeshDatum(elem, side, assembly.kokkosMesh()),
    _assembly(assembly),
    _systems(systems),
    _n_qps(!isSide() ? assembly.getNumQps(_elem) : assembly.getNumFaceQps(_elem, side)),
    _qp_offset(!isSide() ? assembly.getQpOffset(_elem) : assembly.getQpFaceOffset(_elem, side)),
    _elem_property_idx(!isSide() ? _elem.id - _mesh.getStartingContiguousElementID(_elem.subdomain)
                                 : assembly.getElemFacePropertyIndex(_elem, _side))
{
}

KOKKOS_FUNCTION inline Datum::Datum(const ContiguousNodeID node,
                                    const Assembly & assembly,
                                    const Array<FESystem> & systems)
  : MeshDatum(libMesh::DofObject::invalid_id, libMesh::invalid_uint, assembly.kokkosMesh()),
    _assembly(assembly),
    _systems(systems),
    _node(node)
{
}

KOKKOS_FUNCTION inline dof_id_type
Datum::propertyIdx(const PropertyConstantOption constant_option, const unsigned int qp) const
{
  dof_id_type idx = 0;

  if (constant_option == PropertyConstantOption::NONE)
    idx = _qp_offset + qp;
  else if (constant_option == PropertyConstantOption::ELEMENT)
    idx = _elem_property_idx;

  return idx;
}

KOKKOS_FUNCTION inline bool
Datum::isNodalDefined(const Variable & var) const
{
  if (!isNodal() || !var.nodal())
    return false;

  return _systems[var.sys()].isNodalDefined(_node, var.var());
}

KOKKOS_FUNCTION inline const Real33 &
Datum::J(const unsigned int qp)
{
  if (!isNodal())
    reinitTransform(qp);
  else
    _J.identity(_assembly.getDimension());

  return _J;
}

KOKKOS_FUNCTION inline Real
Datum::JxW(const unsigned int qp)
{
  if (!isNodal())
    reinitTransform(qp);
  else
    _JxW = 1;

  return _JxW;
}

KOKKOS_FUNCTION inline Real3
Datum::q_point(const unsigned int qp)
{
  if (!isNodal())
    reinitTransform(qp);
  else
    _xyz = _assembly.kokkosMesh().getNodePoint(_node);

  return _xyz;
}

KOKKOS_FUNCTION inline Real3
Datum::normals(const unsigned int qp)
{
  KOKKOS_ASSERT(isSide());

  if (isSide())
    reinitTransform(qp);

  return _normal;
}

KOKKOS_FUNCTION inline void
Datum::reinitTransform(const unsigned int qp)
{
  if (_cached_qp == qp)
    return;

  if (!isSide())
  {
    _J = _assembly.getJacobian(_elem, qp);
    _JxW = _assembly.getJxW(_elem, qp);
    _xyz = _assembly.getQPoint(_elem, qp);
  }
  else
    _assembly.computePhysicalMap(_elem, _side, qp, &_J, &_JxW, &_xyz, &_normal);

  _cached_qp = qp;
}

/**
 * The Kokkos object that holds thread-private data in the parallel operations of Kokkos kernels
 */
class AssemblyDatum : public Datum
{
public:
  /**
   * Constructor for element and side data
   * @param elem The contiguous element ID of the current thread
   * @param side The side index of the current thread
   * @param assembly The Kokkos assembly
   * @param systems The Kokkos systems
   * @param ivar The Kokkos variable
   * @param jvar The coupled variable number
   * @param comp The variable component
   */
  KOKKOS_FUNCTION
  AssemblyDatum(const ContiguousElementID elem,
                const unsigned int side,
                const Assembly & assembly,
                const Array<FESystem> & systems,
                const Variable & ivar,
                const unsigned int jvar,
                const unsigned int comp = 0)
    : Datum(elem, side, assembly, systems),
      _tag(ivar.tag()),
      _sys(ivar.sys(comp)),
      _ivar(ivar.var(comp)),
      _jvar(jvar),
      _ife(systems[ivar.sys(comp)].getFETypeID(_ivar)),
      _jfe(systems[ivar.sys(comp)].getFETypeID(_jvar)),
      _n_idofs(assembly.getNumDofs(_elem.type, _ife)),
      _n_jdofs(assembly.getNumDofs(_elem.type, _jfe))
  {
  }
  /**
   * Constructor for node data
   * @param elem The contiguous element ID of the current thread
   * @param assembly The Kokkos assembly
   * @param systems The Kokkos systems
   * @param ivar The Kokkos variable
   * @param jvar The coupled variable number
   * @param comp The variable component
   */
  KOKKOS_FUNCTION
  AssemblyDatum(const ContiguousNodeID node,
                const Assembly & assembly,
                const Array<FESystem> & systems,
                const Variable & ivar,
                const unsigned int jvar,
                const unsigned int comp = 0)
    : Datum(node, assembly, systems),
      _tag(ivar.tag()),
      _sys(ivar.sys(comp)),
      _ivar(ivar.var(comp)),
      _jvar(jvar),
      _ife(systems[ivar.sys(comp)].getFETypeID(_ivar)),
      _jfe(systems[ivar.sys(comp)].getFETypeID(_jvar))
  {
  }

  /**
   * Get the number of local DOFs
   * @returns The number of local DOFs
   */
  KOKKOS_FUNCTION unsigned int n_dofs() const { return _n_idofs; }
  /**
   * Get the number of local DOFs
   * @returns The number of local DOFs
   */
  KOKKOS_FUNCTION unsigned int n_idofs() const { return _n_idofs; }
  /**
   * Get the number of local DOFs for the coupled variable
   * @returns The number of local DOFs
   */
  KOKKOS_FUNCTION unsigned int n_jdofs() const { return _n_jdofs; }
  /**
   * Get the system number of variable
   * @returns The system number of variable
   */
  KOKKOS_FUNCTION unsigned int sys() const { return _sys; }
  /**
   * Get the variable number
   * @returns The variable number
   */
  KOKKOS_FUNCTION unsigned int var() const { return _ivar; }
  /**
   * Get the variable number
   * @returns The variable number
   */
  KOKKOS_FUNCTION unsigned int ivar() const { return _ivar; }
  /**
   * Get the coupled variable number
   * @returns The variable number
   */
  KOKKOS_FUNCTION unsigned int jvar() const { return _jvar; }
  /**
   * Get the variable FE type ID
   * @returns The variable FE type ID
   */
  KOKKOS_FUNCTION unsigned int fe() const { return _ife; }
  /**
   * Get the variable FE type ID
   * @returns The variable FE type ID
   */
  KOKKOS_FUNCTION unsigned int ife() const { return _ife; }
  /**
   * Get the coupled variable FE type ID
   * @returns The variable FE type ID
   */
  KOKKOS_FUNCTION unsigned int jfe() const { return _jfe; }
  /**
   * Set whether to compute derivatives for automatic differentiation (AD)
   * @param flag Whether to compute derivatives
   */
  KOKKOS_FUNCTION void do_derivatives(const bool flag) { _do_derivatives = flag; }
  /**
   * Get whether to compute derivatives for automatic differentiation (AD)
   * @returns Whether to compute derivatives
   */
  KOKKOS_FUNCTION bool do_derivatives() const { return _do_derivatives; }

protected:
  /**
   * Solution tag ID
   */
  const TagID _tag;
  /**
   * System number
   */
  const unsigned int _sys;
  /**
   * Variable numbers
   */
  const unsigned int _ivar, _jvar;
  /**
   * FE type IDs of variables
   */
  const unsigned int _ife, _jfe;
  /**
   * Number of local DOFs
   */
  const unsigned int _n_idofs = 1, _n_jdofs = 1;
  /**
   * Whether to compute derivatives for automatic differentiation (AD)
   */
  bool _do_derivatives = true;
};

} // namespace Moose::Kokkos

using Datum = Moose::Kokkos::Datum;
using AssemblyDatum = Moose::Kokkos::AssemblyDatum;
using FVDatum = Moose::Kokkos::FVDatum;
