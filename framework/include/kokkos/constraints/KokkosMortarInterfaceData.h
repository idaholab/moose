//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosArray.h"
#include "KokkosTypes.h"
#include "KokkosMesh.h"
#include "MortarConsumerInterface.h"
#include "MortarExecutorInterface.h"

#include "libmesh/enum_elem_type.h"

#ifdef MOOSE_KOKKOS_SCOPE
#include "libmesh/fe_dual_coeffs.h"
#include "libmesh/kokkos_fe_evaluator.h"
#endif

class AutomaticMortarGeneration;
class FEProblemBase;

namespace Moose::Kokkos
{

/**
 * The sides of a mortar interface whose elements a Kokkos mortar object reads at a segment
 * quadrature point: the interior parents of the secondary and primary faces, on which the primal
 * variables live, and the lower-dimensional secondary element, on which the Lagrange multiplier
 * lives
 */
enum class MortarSide : unsigned int
{
  Secondary = 0,
  Primary = 1,
  Lower = 2
};

/**
 * The geometry of one mortar interface on the device: the quadrature points of the mortar segments
 * of the local secondary elements, and the nodal normals of the secondary surface with their
 * derivatives with respect to the positions of the secondary face nodes. Copies share the data, so
 * a Kokkos object copies the geometry before each dispatch.
 */
struct MortarInterfaceGeometry
{
  /// Maximum number of nodes of a lower-dimensional secondary element
  static constexpr unsigned int max_face_nodes = 9;
  /// Maximum number of shape functions of a variable on a lower-dimensional element
  static constexpr unsigned int max_shapes = 9;

  /**
   * Mortar segment quadrature point data, ordered by local secondary element
   */
  ///@{
  /// Range of the quadrature points of each local secondary element
  Array<dof_id_type> secondary_qp_begin;
  /// Contiguous ID of the element of each side at each point, indexed by MortarSide
  Array<ContiguousElementID> elem[3];
  /// Element type of the element of each side at each point
  Array<libMesh::ElemType> type[3];
  /// Reference coordinates of each point on the element of each side
  Array<Real3> xi[3];
  /// Quadrature weight of each point, including the coordinate transform factor
  Array<Real> JxW;
  /// Gap vector at each point, from the secondary to the primary face
  Array<Real3> gap_vector;
  /// Interpolated secondary normal at each point
  Array<Real3> normal;
  ///@}

  /**
   * Nodal normals of the nodes of the local secondary elements
   */
  ///@{
  /// Index of the nodal normal of each node of each local secondary element: entry
  /// max_face_nodes * e + k for node k of local secondary element e
  Array<dof_id_type> elem_node_normal;
  /// Contiguous ID of the node of each nodal normal
  Array<ContiguousNodeID> normal_node;
  /// Normalized nodal normal
  Array<Real3> nodal_normal;
  /// Range of the sensitivities of each nodal normal
  Array<dof_id_type> sensitivity_begin;
  /// The node whose position each sensitivity is with respect to, as a secondary interior parent
  /// element containing it and the local index of the node on that element, through which the
  /// degrees of freedom of a nodal variable at the node are found
  Array<ContiguousElementID> sensitivity_elem;
  Array<unsigned int> sensitivity_elem_node;
  /// Derivative of the nodal normal with respect to the position of that node: entry (i, c) is
  /// d n_i / d x_c
  Array<Real33> sensitivity;
  ///@}

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Evaluate a shape function on the element of a side at a quadrature point
   * @param family The FE family
   * @param order The FE order
   * @param side The side, a MortarSide
   * @param i The shape function index
   * @param qp The quadrature point
   * @returns The shape function value
   */
  KOKKOS_FUNCTION Real shape(const libMesh::FEFamily family,
                             const libMesh::Order order,
                             const unsigned int side,
                             const unsigned int i,
                             const dof_id_type qp) const
  {
    const auto & x = xi[side][qp];
    const libMesh::FEShapeKey key{family, type[side][qp], order};
    return libMesh::Kokkos::shape(key, i, x(0), x(1), x(2));
  }

  /**
   * Compute the coefficients of the dual basis of a variable on a local secondary element from its
   * segment quadrature, with the routine libMesh uses on the host
   * @param family The FE family of the variable
   * @param order The FE order of the variable
   * @param secondary The index of the local secondary element
   * @param n_shapes The number of shape functions of the variable on the element
   * @param coeff The coefficients: dual function i is sum_k coeff[k][i] phi_k
   */
  KOKKOS_FUNCTION void dualCoefficients(const libMesh::FEFamily family,
                                        const libMesh::Order order,
                                        const dof_id_type secondary,
                                        const unsigned int n_shapes,
                                        Real (&coeff)[max_shapes][max_shapes]) const
  {
    constexpr auto L = static_cast<unsigned int>(MortarSide::Lower);
    const auto begin = secondary_qp_begin[secondary];
    const auto end = secondary_qp_begin[secondary + 1];

    Real A[max_shapes][max_shapes], D[max_shapes][max_shapes], rhs[max_shapes], x[max_shapes];
    auto A_ref = [&A](const unsigned int i, const unsigned int j) -> Real & { return A[i][j]; };
    auto D_ref = [&D](const unsigned int i, const unsigned int j) -> Real & { return D[i][j]; };
    auto rhs_ref = [&rhs](const unsigned int i) -> Real & { return rhs[i]; };
    auto x_ref = [&x](const unsigned int i) -> Real & { return x[i]; };
    auto coeff_ref = [&coeff](const unsigned int i, const unsigned int j) -> Real &
    { return coeff[i][j]; };
    const auto weight = [&](const unsigned int qp) { return JxW[begin + qp]; };
    const auto phi = [&](const unsigned int i, const unsigned int qp)
    { return shape(family, order, L, i, begin + qp); };

    libMesh::compute_dual_coeffs(family,
                                 type[L][begin],
                                 n_shapes,
                                 end - begin,
                                 weight,
                                 phi,
                                 A_ref,
                                 D_ref,
                                 rhs_ref,
                                 x_ref,
                                 coeff_ref);
  }

  /**
   * Evaluate a basis function of a variable on the lower-dimensional secondary element at a
   * quadrature point: the dual basis function when coefficients are given
   * @param family The FE family of the variable
   * @param order The FE order of the variable
   * @param coeff The dual basis coefficients from dualCoefficients(), or nullptr for the standard
   * basis
   * @param n_shapes The number of shape functions of the variable on the element
   * @param i The basis function index
   * @param qp The quadrature point
   * @returns The basis function value
   */
  KOKKOS_FUNCTION Real lowerBasis(const libMesh::FEFamily family,
                                  const libMesh::Order order,
                                  const Real (*coeff)[max_shapes],
                                  const unsigned int n_shapes,
                                  const unsigned int i,
                                  const dof_id_type qp) const
  {
    constexpr auto L = static_cast<unsigned int>(MortarSide::Lower);
    if (!coeff)
      return shape(family, order, L, i, qp);

    Real psi = 0;
    for (unsigned int k = 0; k < n_shapes; ++k)
      psi += coeff[k][i] * shape(family, order, L, k, qp);
    return psi;
  }
#endif
};

/**
 * The host-side mortar interface of a Kokkos object, which the device copies of the object do not
 * carry
 */
class MortarConsumer : public MortarConsumerInterface
{
public:
  MortarConsumer(const MooseObject * object) : MortarConsumerInterface(object) {}

  using MortarConsumerInterface::amg;
};

/**
 * The host owner of the geometry of one mortar interface, shared by the Kokkos objects posed on it
 * so the geometry is packed once per change of the mortar segment mesh
 */
class MortarInterfaceData : public MortarExecutorInterface
{
public:
  /**
   * Constructor
   * @param problem The problem
   * @param amg The mortar interface
   */
  MortarInterfaceData(FEProblemBase & problem, const AutomaticMortarGeneration & amg);

  virtual void mortarSetup(const AutomaticMortarGeneration & amg) override;

  /// Request the nodal normals and their sensitivities, which only some objects read
  void requireNodalNormals()
  {
    _needs_packing |= !_need_normals;
    _need_normals = true;
  }

  /**
   * Pack the geometry if the mortar segment mesh was set up or rebuilt since the last packing
   * @param sensitivities Whether the nodal normal sensitivities are needed, which only a Jacobian
   * evaluation reads
   * @returns The geometry
   */
  const MortarInterfaceGeometry & update(bool sensitivities = false);

  /// The mortar interface
  const AutomaticMortarGeneration & amg() const { return _amg; }

  /// The number of times the geometry was packed, by which a consumer tells whether its own data
  /// derived from the interface is current
  unsigned int revision() const { return _revision; }

  /// The reference mesh nodes of the nodal normals, in the order of the packed normals
  const std::vector<const Node *> & normalNodes() const { return _normal_nodes; }

private:
  /// Pack the mortar segment quadrature points of the local secondary elements
  void packSegments();

  /// Pack the nodal normals of the nodes of the local secondary elements
  void packNodalNormals();

  /// Pack the derivatives of the nodal normals with respect to the secondary face node positions
  void packSensitivities();

  /// The problem
  FEProblemBase & _problem;

  /// The mortar interface
  const AutomaticMortarGeneration & _amg;

  /// The packed geometry
  MortarInterfaceGeometry _geometry;

  /// Local secondary elements in the order of the packed geometry
  std::vector<const Elem *> _local_secondary_elems;
  /// The same elements on the mesh of the mortar interface, the displaced mesh on the current
  /// configuration
  std::vector<const Elem *> _amg_secondary_elems;

  /// The nodes of the packed nodal normals
  std::vector<const Node *> _normal_nodes;

  /// Whether the geometry must be repacked before the next use
  bool _needs_packing = true;

  /// Whether any object reads the nodal normals
  bool _need_normals = false;

  /// The number of times the geometry was packed
  unsigned int _revision = 0;

  /// The build count of the mortar segment mesh as of the last packing
  unsigned int _packed_build_count = 0;

  /// The revision of the geometry that the sensitivities were packed for
  unsigned int _sensitivity_revision = 0;
};

} // namespace Moose::Kokkos
