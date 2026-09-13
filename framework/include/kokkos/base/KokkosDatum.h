//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDatumDecl.h"

#include "KokkosAssembly.h"
#include "KokkosFESystem.h"
#include "KokkosVariable.h"
#include "KokkosMaterialPropertyDecl.h"

namespace Moose::Kokkos
{

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

KOKKOS_FUNCTION inline FVDatum::FVDatum(ContiguousElementID elem,
                                        const unsigned int side,
                                        const Mesh & mesh)
  : MeshDatum(elem, side, mesh)
{
}

KOKKOS_FUNCTION inline const FESystem &
Datum::system(const unsigned int sys) const
{
  return _systems[sys];
}

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

KOKKOS_FUNCTION inline AssemblyDatum::AssemblyDatum(const ContiguousElementID elem,
                                                    const unsigned int side,
                                                    const Assembly & assembly,
                                                    const Array<FESystem> & systems,
                                                    const Variable & ivar,
                                                    const unsigned int jvar,
                                                    const unsigned int comp)
  : Datum(elem, side, assembly, systems),
    _tag(ivar.tag()),
    _sys(ivar.sys(comp)),
    _ivar(ivar.var(comp)),
    _jvar(jvar),
    _comp(comp),
    _ife(systems[ivar.sys(comp)].getFETypeID(_ivar)),
    _jfe(systems[ivar.sys(comp)].getFETypeID(_jvar)),
    _n_idofs(assembly.getNumDofs(_elem.type, _ife)),
    _n_jdofs(assembly.getNumDofs(_elem.type, _jfe))
{
}

KOKKOS_FUNCTION inline AssemblyDatum::AssemblyDatum(const ContiguousNodeID node,
                                                    const Assembly & assembly,
                                                    const Array<FESystem> & systems,
                                                    const Variable & ivar,
                                                    const unsigned int jvar,
                                                    const unsigned int comp)
  : Datum(node, assembly, systems),
    _tag(ivar.tag()),
    _sys(ivar.sys(comp)),
    _ivar(ivar.var(comp)),
    _jvar(jvar),
    _comp(comp),
    _ife(systems[ivar.sys(comp)].getFETypeID(_ivar)),
    _jfe(systems[ivar.sys(comp)].getFETypeID(_jvar))
{
}

} // namespace Moose::Kokkos
