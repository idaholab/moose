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

namespace Moose
{
namespace Kokkos
{

/**
 * The Kokkos object that holds thread-private data in the parallel operations of any Kokkos object
 */
class Datum
{
public:
  /**
   * Constructor
   * @param elem The element ID of the current thread
   * @param side The side index of the current thread
   * @param assembly The Kokkos assembly
   * @param systems The Kokkos systems
   */
  KOKKOS_FUNCTION
  Datum(const dof_id_type elem,
        const unsigned int side,
        const Assembly & assembly,
        const Array<System> & systems)
    : _assembly(assembly),
      _systems(systems),
      _elem(assembly.kokkosMesh().getElementInfo(elem)),
      _side(side),
      _neighbor(_side != libMesh::invalid_uint ? assembly.kokkosMesh().getNeighbor(_elem.id, side)
                                               : libMesh::DofObject::invalid_id),
      _n_qps(side == libMesh::invalid_uint ? assembly.getNumQps(_elem)
                                           : assembly.getNumFaceQps(_elem, side)),
      _qp_offset(side == libMesh::invalid_uint ? assembly.getQpOffset(_elem)
                                               : assembly.getQpFaceOffset(_elem, side))
  {
  }
  /**
   * Constructor for elemental data
   * @param elem The element ID of the current thread
   * @param assembly The Kokkos assembly
   * @param systems The Kokkos systems
   */
  KOKKOS_FUNCTION
  Datum(const dof_id_type elem, const Assembly & assembly, const Array<System> & systems)
    : Datum(elem, libMesh::invalid_uint, assembly, systems)
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
   * Get the subdomain ID
   * @returns The subdomain ID
   */
  KOKKOS_FUNCTION SubdomainID subdomain() const { return _elem.subdomain; }
  /**
   * Get the side index
   * @returns The side index
   */
  KOKKOS_FUNCTION unsigned int side() const { return _side; }
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
   * Get whether the current side has a neighbor
   * @returns Whether the current side has a neighbor
   */
  KOKKOS_FUNCTION bool hasNeighbor() const { return _neighbor != libMesh::DofObject::invalid_id; }

  /**
   * Get the inverse of Jacobian matrix
   * | dxi/dx deta/dx dzeta/dx |
   * | dxi/dy deta/dy dzeta/dy |
   * | dxi/dz deta/dz dzeta/dz |
   * @param qp The local quadrature point index
   * @returns The Jacobian matrix
   */
  KOKKOS_FUNCTION const Real33 & J(const unsigned int qp)
  {
    reinitTransform(qp);

    return _J;
  }
  /**
   * Get the transformed Jacobian weight
   * @param qp The local quadrature point index
   * @returns The transformed Jacobian weights
   */
  KOKKOS_FUNCTION Real JxW(const unsigned int qp)
  {
    reinitTransform(qp);

    return _JxW;
  }
  /**
   * Get the physical quadrature point coordinate
   * @param qp The local quadrature point index
   * @returns The physical quadrature point coordinate
   */
  KOKKOS_FUNCTION Real3 q_point(const unsigned int qp)
  {
    reinitTransform(qp);

    return _xyz;
  }

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
  const unsigned int _side;
  /**
   * Current element ID of neighbor
   */
  const dof_id_type _neighbor;
  /**
   * Number of local quadrature points
   */
  const unsigned int _n_qps;
  /**
   * Starting offset into the global quadrature point index
   */
  const dof_id_type _qp_offset;

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
 * The Kokkos object that holds thread-private data in the parallel operations of Kokkos residual
 * objects
 */
class ResidualDatum : public Datum
{
public:
  /**
   * Constructor
   * @param elem The element ID of the current thread
   * @param side The side index of the current thread
   * @param assembly The Kokkos assembly
   * @param systems The Kokkos systems
   * @param ivar The Kokkos variable
   * @param jvar The coupled variable number
   * @param comp The variable component
   */
  KOKKOS_FUNCTION
  ResidualDatum(const dof_id_type elem,
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
   * Constructor for elemental data
   * @param elem The element ID of the current thread
   * @param assembly The Kokkos assembly
   * @param systems The Kokkos systems
   * @param ivar The Kokkos variable
   * @param jvar The coupled variable number
   * @param comp The variable component
   */
  KOKKOS_FUNCTION
  ResidualDatum(const dof_id_type elem,
                const Assembly & assembly,
                const Array<System> & systems,
                const Variable & ivar,
                const unsigned int jvar,
                const unsigned int comp = 0)
    : ResidualDatum(elem, libMesh::invalid_uint, assembly, systems, ivar, jvar, comp)
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
  const unsigned int _n_idofs, _n_jdofs;
};

} // namespace Kokkos
} // namespace Moose

using Datum = Moose::Kokkos::Datum;
using ResidualDatum = Moose::Kokkos::ResidualDatum;
