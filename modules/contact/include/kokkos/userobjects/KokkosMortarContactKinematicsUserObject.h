//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosGeneralUserObject.h"
#include "KokkosMortarInterfaceData.h"

/**
 * Computes the weighted gap vector of each secondary node j of a mortar contact interface,
 *
 *   A_j = sum_qp psi_j (x_primary - x_secondary) JxW,
 *
 * its weighted gap g_j = A_j . n_j,
 * and its normalization sum_qp psi_j JxW over the mortar segment quadrature points, with psi the
 * basis of the Lagrange multiplier (its dual basis when it uses one) and n_j the nodal normal.
 * With compute_slip, it also integrates the weighted relative displacement increment over the time
 * step, sum_qp psi_j (du_secondary - du_primary) JxW with du = u - u_old.
 * Each process integrates its local secondary elements, the owner of a node sums the contributions
 * of all processes, and every process whose local secondary elements touch the node receives the
 * sums. The owner also designates one of those processes to assemble the nodal terms of the node.
 * The quantities are geometric: the constraints that read them decide the contact conditions.
 */
class KokkosMortarContactKinematicsUserObject : public Moose::Kokkos::GeneralUserObject
{
public:
  static InputParameters validParams();

  KokkosMortarContactKinematicsUserObject(const InputParameters & parameters);
  KokkosMortarContactKinematicsUserObject(const KokkosMortarContactKinematicsUserObject & object);

  virtual void initialize() override {}
  virtual void compute() override;
  virtual void finalize() override {}

  /// Kokkos function tag of the integration, one team per local secondary element
  struct GapLoop
  {
  };

  /**
   * The team operation called by Kokkos for each local secondary element
   */
  KOKKOS_FUNCTION void operator()(GapLoop,
                                  const Moose::Kokkos::TeamMember<GapLoop> & team,
                                  const KokkosMortarContactKinematicsUserObject &) const;

  /// The weighted gap of each nodal normal of the interface geometry, summed over all processes
  const Moose::Kokkos::Array<Real> & weightedGap() const { return _gap; }
  /// The weighted gap vector sum_qp psi_j (x_primary - x_secondary) JxW of each nodal normal,
  /// summed over all processes
  const Moose::Kokkos::Array<Moose::Kokkos::Real3> & weightedGapVector() const
  {
    return _gap_vector;
  }
  /// The weighted slip of each nodal normal of the interface geometry, summed over all processes
  const Moose::Kokkos::Array<Moose::Kokkos::Real3> & weightedSlip() const { return _slip; }
  /// The normalization of each nodal normal of the interface geometry, summed over all processes
  const Moose::Kokkos::Array<Real> & normalization() const { return _normalization; }

  /// The IDs of the secondary nodes this process owns and some mortar segment reaches, in
  /// ascending order
  const std::vector<dof_id_type> & ownedNodes() const { return _owned_nodes; }
  /// The weighted gap and the normalization of each owned node
  const Moose::Kokkos::Array<Real> & ownedWeightedGap() const { return _owned_gap; }
  const Moose::Kokkos::Array<Real> & ownedNormalization() const { return _owned_normalization; }
  /// Whether this process assembles the nodal terms of each nodal normal of the interface
  /// geometry: exactly one of the processes whose local secondary elements touch a node does
  const Moose::Kokkos::Array<bool> & designated() const { return _designated; }
  /// The number of times the owned nodes changed
  unsigned int ownedRevision() const { return _owned_revision; }

private:
  /// Sum the contributions of all processes to the nodal quantities
  void communicate();

  /// The host-side mortar interface, shared by the copies of the object
  std::shared_ptr<Moose::Kokkos::MortarConsumer> _interface;

  /// The geometry of the interface, shared by the Kokkos objects posed on it
  Moose::Kokkos::MortarInterfaceData & _interface_data;

  /// The geometry of the interface as of the current computation
  Moose::Kokkos::MortarInterfaceGeometry _geometry;

  /// The Lagrange multiplier, whose basis weights the gap
  const MooseVariableFieldBase & _lm_var;
  /// FE type ID, family, and order of the Lagrange multiplier
  unsigned int _lm_fe = 0;
  const libMesh::FEFamily _lm_family;
  const libMesh::Order _lm_order;
  /// Whether the Lagrange multiplier uses a dual basis
  const bool _use_dual;

  /// Whether the weighted slip is computed
  const bool _compute_slip;
  /// The displacements and their values at the start of the time step
  Moose::Kokkos::Variable _disp;
  Moose::Kokkos::Variable _disp_old;
  /// FE type ID, family, and order of the displacements
  unsigned int _disp_fe = 0;
  libMesh::FEFamily _disp_family = libMesh::LAGRANGE;
  libMesh::Order _disp_order = libMesh::FIRST;

  /// The nodal quantities, indexed by nodal normal
  Moose::Kokkos::Array<Real> _gap;
  Moose::Kokkos::Array<Moose::Kokkos::Real3> _gap_vector;
  Moose::Kokkos::Array<Real> _normalization;
  Moose::Kokkos::Array<Moose::Kokkos::Real3> _slip;
  Moose::Kokkos::Array<bool> _designated;

  /// The owned nodes and their quantities
  std::vector<dof_id_type> _owned_nodes;
  Moose::Kokkos::Array<Real> _owned_gap;
  Moose::Kokkos::Array<Real> _owned_normalization;
  unsigned int _owned_revision = 0;

  /// The dispatcher of the integration
  std::unique_ptr<Moose::Kokkos::DispatcherBase> _gap_dispatcher;
};

KOKKOS_FUNCTION inline void
KokkosMortarContactKinematicsUserObject::operator()(
    GapLoop,
    const Moose::Kokkos::TeamMember<GapLoop> & team,
    const KokkosMortarContactKinematicsUserObject &) const
{
  constexpr auto L = static_cast<unsigned int>(Moose::Kokkos::MortarSide::Lower);
  constexpr auto max_face_nodes = Moose::Kokkos::MortarInterfaceGeometry::max_face_nodes;
  constexpr auto max_shapes = Moose::Kokkos::MortarInterfaceGeometry::max_shapes;

  const auto secondary = team.league_rank();
  const auto begin = _geometry.secondary_qp_begin[secondary];
  const auto end = _geometry.secondary_qp_begin[secondary + 1];
  const auto n_lambda = kokkosAssembly().getNumDofs(
      kokkosMesh().getElementInfo(_geometry.elem[L][begin]).type, _lm_fe);

  Real dual_coeff[max_shapes][max_shapes];
  if (_use_dual)
    _geometry.dualCoefficients(_lm_family, _lm_order, secondary, n_lambda, dual_coeff);

  for (dof_id_type qp = begin; qp < end; ++qp)
  {
    // The relative displacement increment of the secondary face with respect to the primary face
    Moose::Kokkos::Real3 du;
    if (_compute_slip)
      for (const unsigned int face : {0u, 1u})
      {
        const auto elem = _geometry.elem[face][qp];
        const auto n_disp_dofs =
            kokkosAssembly().getNumDofs(kokkosMesh().getElementInfo(elem).type, _disp_fe);
        const Real sign = face == 0 ? 1 : -1;
        for (unsigned int c = 0; c < _disp.components(); ++c)
        {
          const auto & sys = kokkosSystem(_disp.sys(c));
          for (unsigned int j = 0; j < n_disp_dofs; ++j)
          {
            const auto dof = sys.getElemLocalDofIndex(elem, j, _disp.var(c));
            du(c) += sign *
                     (sys.getVectorDofValue(dof, _disp.tag()) -
                      sys.getVectorDofValue(dof, _disp_old.tag())) *
                     _geometry.shape(_disp_family, _disp_order, face, j, qp);
          }
        }
      }

    ::Kokkos::parallel_for(
        ::Kokkos::TeamThreadRange(team, n_lambda),
        [&](const unsigned int k)
        {
          // The shape functions of a Lagrange multiplier follow the nodes of the element
          const auto normal = _geometry.elem_node_normal[max_face_nodes * secondary + k];
          const auto & d = _geometry.gap_vector[qp];
          const auto w =
              _geometry.lowerBasis(
                  _lm_family, _lm_order, _use_dual ? dual_coeff : nullptr, n_lambda, k, qp) *
              _geometry.JxW[qp];

          for (unsigned int c = 0; c < 3; ++c)
            ::Kokkos::atomic_add(&_gap_vector[normal](c), w * d(c));
          ::Kokkos::atomic_add(&_normalization[normal], w);
          if (_compute_slip)
            for (unsigned int c = 0; c < 3; ++c)
              ::Kokkos::atomic_add(&_slip[normal](c), w * du(c));
        });
  }
}
