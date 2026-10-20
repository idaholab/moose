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
#include "MortarExecutorInterface.h"

#include "libmesh/enum_elem_type.h"

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
  /// Contiguous ID of the node whose position each sensitivity is with respect to
  Array<ContiguousNodeID> sensitivity_node;
  /// Derivative of the nodal normal with respect to the position of that node: entry (i, c) is
  /// d n_i / d x_c
  Array<Real33> sensitivity;
  ///@}
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
  void requireNodalNormals() { _need_normals = true; }

  /**
   * Pack the geometry if the mortar segment mesh changed since the last packing, or on every call
   * for an interface on the displaced mesh, whose segments move with the displacements
   * @returns The geometry
   */
  const MortarInterfaceGeometry & update();

  /// The mortar interface
  const AutomaticMortarGeneration & amg() const { return _amg; }

  /// The number of times the geometry was packed, by which a consumer tells whether its own data
  /// derived from the interface is current
  unsigned int revision() const { return _revision; }

private:
  /// Pack the mortar segment quadrature points of the local secondary elements
  void packSegments();

  /// Pack the nodal normals and their sensitivities
  void packNodalNormals();

  /// The problem
  FEProblemBase & _problem;

  /// The mortar interface
  const AutomaticMortarGeneration & _amg;

  /// The packed geometry
  MortarInterfaceGeometry _geometry;

  /// Local secondary elements in the order of the packed geometry
  std::vector<const Elem *> _local_secondary_elems;

  /// Whether the geometry must be repacked before the next use
  bool _needs_packing = true;

  /// Whether any object reads the nodal normals
  bool _need_normals = false;

  /// The number of times the geometry was packed
  unsigned int _revision = 0;
};

} // namespace Moose::Kokkos
