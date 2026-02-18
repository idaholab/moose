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
#include "KokkosSystem.h"
#include "KokkosVariable.h"

namespace Moose::Kokkos
{

/**
 * The Kokkos object that holds thread-private data in the parallel operations of any Kokkos object
 */
class Datum
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
        const Array<System> & systems)
    : _assembly(assembly),
      _systems(systems),
      _elem(assembly.kokkosMesh().getElementInfo(elem)),
      _side(side),
      _neighbor(_side == libMesh::invalid_uint ? libMesh::DofObject::invalid_id
                                               : assembly.kokkosMesh().getNeighbor(_elem.id, side)),
      _n_qps(side == libMesh::invalid_uint ? assembly.getNumQps(_elem)
                                           : assembly.getNumFaceQps(_elem, side)),
      _qp_offset(side == libMesh::invalid_uint ? assembly.getQpOffset(_elem)
                                               : assembly.getQpFaceOffset(_elem, side)),
      _elem_property_idx(
          _side == libMesh::invalid_uint
              ? _elem.id - assembly.kokkosMesh().getStartingContiguousElementID(_elem.subdomain)
              : assembly.getElemFacePropertyIndex(_elem, _side))
  {
  }
  /**
   * Constructor for node data
   * @param node The contiguous node ID of the current thread
   * @param assembly The Kokkos assembly
   * @param systems The Kokkos systems
   */
  KOKKOS_FUNCTION
  Datum(const ContiguousNodeID node, const Assembly & assembly, const Array<System> & systems)
    : _assembly(assembly), _systems(systems), _node(node)
  {
  }

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
  KOKKOS_FUNCTION const System & system(unsigned int sys) const { return _systems[sys]; }

  /**
   * Get the element information object
   * @returns The element information object
   */
  KOKKOS_FUNCTION const ElementInfo & elem() const { return _elem; }
  /**
   * Get the contiguous subdomain ID
   * @returns The contiguous subdomain ID
   */
  KOKKOS_FUNCTION ContiguousSubdomainID subdomain() const { return _elem.subdomain; }
  /**
   * Get the side index
   * @returns The side index
   */
  KOKKOS_FUNCTION unsigned int side() const { return _side; }
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
   * Get whether the current side has a neighbor
   * @returns Whether the current side has a neighbor
   */
  KOKKOS_FUNCTION bool hasNeighbor() const { return _neighbor != libMesh::DofObject::invalid_id; }
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
   * Reset the reinit flag
   */
  KOKKOS_FUNCTION void reinit() { _transform_reinit = false; }

protected:
  /**
   * Reference of the Kokkos assembly
   */
  const Assembly & _assembly;
  /**
   * Reference of the Kokkos systems
   */
  const Array<System> & _systems;
  /**
   * Current element information object
   */
  const ElementInfo _elem;
  /**
   * Current side index
   */
  const unsigned int _side = libMesh::invalid_uint;
  /**
   * Current contiguous node ID
   */
  const ContiguousNodeID _node = libMesh::DofObject::invalid_id;
  /**
   * Current contiguous element ID of neighbor
   */
  const ContiguousElementID _neighbor = libMesh::DofObject::invalid_id;
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
   * Flag whether the physical transformation data was cached
   */
  bool _transform_reinit = false;
  /**
   * Cached physical transformation data
   */
  ///@{
  Real33 _J;
  Real _JxW;
  Real3 _xyz;
  ///@}
};

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

KOKKOS_FUNCTION inline void
Datum::reinitTransform(const unsigned int qp)
{
  if (_transform_reinit)
    return;

  if (_side == libMesh::invalid_uint)
  {
    _J = _assembly.getJacobian(_elem, qp);
    _JxW = _assembly.getJxW(_elem, qp);
    _xyz = _assembly.getQPoint(_elem, qp);
  }
  else
    _assembly.computePhysicalMap(_elem, _side, qp, &_J, &_JxW, &_xyz);

  _transform_reinit = true;
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
                const Array<System> & systems,
                const Variable & ivar,
                const unsigned int jvar,
                const unsigned int comp = 0)
    : Datum(elem, side, assembly, systems),
      _tag(ivar.tag()),
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
                const Array<System> & systems,
                const Variable & ivar,
                const unsigned int jvar,
                const unsigned int comp = 0)
    : Datum(node, assembly, systems),
      _tag(ivar.tag()),
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

protected:
  /**
   * Solution tag ID
   */
  const TagID _tag;
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
};

} // namespace Moose::Kokkos

using Datum = Moose::Kokkos::Datum;
using AssemblyDatum = Moose::Kokkos::AssemblyDatum;
