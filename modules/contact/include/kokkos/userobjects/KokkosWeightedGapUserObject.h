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
 * Computes the weighted gap of each secondary node j of a mortar contact interface,
 *
 *   g_j = sum_qp psi_j (x_primary - x_secondary) . n_j JxW,
 *
 * and its normalization sum_qp psi_j JxW over the mortar segment quadrature points, with psi the
 * basis of the Lagrange multiplier (its dual basis when it uses one) and n_j the nodal normal.
 * Each process integrates its local secondary elements, the owner of a node sums the contributions
 * of all processes, and every process whose local secondary elements touch the node receives the
 * sum. The quantities are geometric: the constraints that read them decide the contact
 * conditions.
 */
class KokkosWeightedGapUserObject : public Moose::Kokkos::GeneralUserObject
{
public:
  static InputParameters validParams();

  KokkosWeightedGapUserObject(const InputParameters & parameters);
  KokkosWeightedGapUserObject(const KokkosWeightedGapUserObject & object);

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
                                  const KokkosWeightedGapUserObject &) const;

  /// The weighted gap of each nodal normal of the interface geometry, summed over all processes
  const Moose::Kokkos::Array<Real> & weightedGap() const { return _gap; }
  /// The normalization of each nodal normal of the interface geometry, summed over all processes
  const Moose::Kokkos::Array<Real> & normalization() const { return _normalization; }

  /// The IDs of the secondary nodes this process owns and some mortar segment reaches, in
  /// ascending order
  const std::vector<dof_id_type> & ownedNodes() const { return _owned_nodes; }
  /// The weighted gap and the normalization of each owned node
  const Moose::Kokkos::Array<Real> & ownedWeightedGap() const { return _owned_gap; }
  const Moose::Kokkos::Array<Real> & ownedNormalization() const { return _owned_normalization; }
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

  /// The nodal quantities, indexed by nodal normal
  Moose::Kokkos::Array<Real> _gap;
  Moose::Kokkos::Array<Real> _normalization;

  /// The owned nodes and their quantities
  std::vector<dof_id_type> _owned_nodes;
  Moose::Kokkos::Array<Real> _owned_gap;
  Moose::Kokkos::Array<Real> _owned_normalization;
  unsigned int _owned_revision = 0;

  /// The dispatcher of the integration
  std::unique_ptr<Moose::Kokkos::DispatcherBase> _gap_dispatcher;
};

KOKKOS_FUNCTION inline void
KokkosWeightedGapUserObject::operator()(GapLoop,
                                        const Moose::Kokkos::TeamMember<GapLoop> & team,
                                        const KokkosWeightedGapUserObject &) const
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
    ::Kokkos::parallel_for(
        ::Kokkos::TeamThreadRange(team, n_lambda),
        [&](const unsigned int k)
        {
          // The shape functions of a Lagrange multiplier follow the nodes of the element
          const auto normal = _geometry.elem_node_normal[max_face_nodes * secondary + k];
          const auto & n = _geometry.nodal_normal[normal];
          const auto & d = _geometry.gap_vector[qp];
          const auto w =
              _geometry.lowerBasis(
                  _lm_family, _lm_order, _use_dual ? dual_coeff : nullptr, n_lambda, k, qp) *
              _geometry.JxW[qp];

          ::Kokkos::atomic_add(&_gap[normal], w * (d(0) * n(0) + d(1) * n(1) + d(2) * n(2)));
          ::Kokkos::atomic_add(&_normalization[normal], w);
        });
}
