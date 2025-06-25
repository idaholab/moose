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
#include "GPUAssembly.h"
#include "GPUSystem.h"
#include "GPUVariable.h"

namespace Moose
{
namespace Kokkos
{

class Datum
{
public:
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
  KOKKOS_FUNCTION
  Datum(const dof_id_type elem, const Assembly & assembly, const Array<System> & systems)
    : Datum(elem, libMesh::invalid_uint, assembly, systems)
  {
  }

  KOKKOS_FUNCTION const auto & elem() const { return _elem; }
  KOKKOS_FUNCTION auto side() const { return _side; }
  KOKKOS_FUNCTION auto n_qps() const { return _n_qps; }
  KOKKOS_FUNCTION auto qpOffset() const { return _qp_offset; }

  KOKKOS_FUNCTION bool hasNeighbor() { return _neighbor != libMesh::DofObject::invalid_id; }

  // Assembly
  KOKKOS_FUNCTION const Real33 & J(const unsigned int qp)
  {
    reinitTransform(qp);

    return _J;
  }
  KOKKOS_FUNCTION Real JxW(const unsigned int qp)
  {
    reinitTransform(qp);

    return _JxW;
  }
  KOKKOS_FUNCTION Real3 q_point(const unsigned int qp)
  {
    reinitTransform(qp);

    return _xyz;
  }
  KOKKOS_FUNCTION void reinit(const unsigned int qp)
  {
    _transform_reinit = false;

    if (_side != libMesh::invalid_uint)
      return;

    bool need_reinit = false;

    for (unsigned int s = 0; s < _systems.size(); ++s)
      need_reinit = need_reinit || _systems[s].needReinit(_elem);

    if (need_reinit)
    {
      reinitTransform(qp);

      for (unsigned int s = 0; s < _systems.size(); ++s)
        _systems[s].reinit(_elem, _J, _qp_offset + qp, qp);
    }
  }

protected:
  KOKKOS_FUNCTION void reinitTransform(const unsigned int qp)
  {
    if (_transform_reinit)
      return;

    if (_side == libMesh::invalid_uint)
      _assembly.computePhysicalMap(_elem, qp, &_J, &_JxW, &_xyz);
    else
      _assembly.computePhysicalMap(_elem, _side, qp, &_J, &_JxW, &_xyz);

    _transform_reinit = true;
  }

protected:
  // Reference to the Kokkos assembly
  const Assembly & _assembly;
  // Reference to the Kokkos systems
  const Array<System> & _systems;
  // Element information
  const ElementInfo _elem;
  // Side index
  const unsigned int _side;
  // Neighbor local element ID
  const dof_id_type _neighbor;
  // Number of quadrature points
  const unsigned int _n_qps;
  // Quadrature point offset
  const dof_id_type _qp_offset;
  // Whether physical trnasformation data was reinit
  bool _transform_reinit = false;
  // Physical transformation
  Real33 _J;
  Real _JxW;
  Real3 _xyz;
};

class ResidualDatum : public Datum
{
public:
  KOKKOS_FUNCTION
  ResidualDatum(const dof_id_type elem,
                const unsigned int side,
                const Assembly & assembly,
                const Array<System> & systems,
                const Variable & ivar,
                const unsigned int jvar,
                const unsigned int comp = 0)
    : Datum(elem, side, assembly, systems),
      _system(systems[ivar.sys(comp)]),
      _tag(ivar.tag()),
      _ivar(ivar.var(comp)),
      _jvar(jvar),
      _ife(_system.getFETypeNum(_ivar)),
      _jfe(_system.getFETypeNum(_jvar)),
      _n_idofs(_assembly.getNumDofs(_elem.type, _ife)),
      _n_jdofs(_assembly.getNumDofs(_elem.type, _jfe))
  {
  }
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

  KOKKOS_FUNCTION auto n_dofs() const { return _n_idofs; }
  KOKKOS_FUNCTION auto n_idofs() const { return _n_idofs; }
  KOKKOS_FUNCTION auto n_jdofs() const { return _n_jdofs; }
  KOKKOS_FUNCTION auto ivar() const { return _ivar; }
  KOKKOS_FUNCTION auto jvar() const { return _jvar; }
  KOKKOS_FUNCTION auto ife() const { return _ife; }
  KOKKOS_FUNCTION auto jfe() const { return _jfe; }
  KOKKOS_FUNCTION bool diag() const { return _ivar == _jvar; }

protected:
  // Solution tag ID
  const TagID _tag;
  // Reference to the Kokkos system
  const System & _system;
  // Variable numbers
  const unsigned int _ivar, _jvar;
  // FE type numbers of variables
  const unsigned int _ife, _jfe;
  // Number of DOFs
  const unsigned int _n_idofs, _n_jdofs;
};

} // namespace Kokkos
} // namespace Moose

using Datum = Moose::Kokkos::Datum;
using ResidualDatum = Moose::Kokkos::ResidualDatum;
