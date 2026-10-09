//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionInterface.h"
#include "MeshSmootherBase.h"

#include "libmesh/sparse_matrix.h"

#include <array>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace libMesh
{
class LinearImplicitSystem;
class Node;
}

class Function;
class MooseVariableFieldBase;

/**
 * Moves the mesh between remesh events by harmonically interpolating the motion of the moving
 * boundaries into the interior.
 *
 * Per spatial component i, the total pseudo-displacement since the last remesh solves
 *
 *     integral over Omega_0 of (grad d_i . grad psi) dOmega = 0
 *
 * with d_i = d_i_prev + dt * v_i on Gamma_mov and d_i = 0 on Gamma_fix, where psi is a linear
 * Lagrange test function, v_i the interface velocity component, and Omega_0 the configuration
 * snapshotted at the last remesh. What is solved for is the total displacement since that
 * snapshot, not an increment: the moving boundary values accumulate, while the interior is a
 * fresh harmonic interpolation of them every step.
 *
 * The operator only depends on Omega_0, so it is assembled once per mesh topology instead of once
 * per time step. The components then differ in their right hand side and in which rows they pin,
 * because a fixed wall that slides constrains only the components its normal points along.
 *
 * The linear system lives in the problem's own EquationSystems rather than in one this object
 * owns, because libMesh stores the degrees of freedom of every system on the mesh's own Node and
 * Elem objects: a second EquationSystems over the same MeshBase resets that storage and destroys
 * the numbering of the user's systems. MOOSE only solves the systems it knows about, so a raw
 * libMesh system added this way is never part of the user's nonlinear problem.
 *
 * Its Krylov solver carries the system name as a PETSc options prefix, so a smoother named
 * 'laplace' is tuned with -laplace_smoother_laplace_pc_type and is not reached by the global
 * -pc_type that configures the user's own solves. That solver is destroyed before every component
 * solve, because zeroing the pinned rows drops their off-diagonal entries and so gives each
 * component its own sparsity: a preconditioner that caches structure, such as asm or redundant,
 * cannot be handed an operator it was not built for.
 */
class LaplaceSmoother : public MeshSmootherBase, public FunctionInterface
{
public:
  static InputParameters validParams();

  LaplaceSmoother(const InputParameters & parameters);

  virtual void updatePseudoDisplacement(Real dt) override;
  virtual void reset() override;
  virtual void reinitOnNewMesh() override;

private:
  /**
   * Resolve the boundaries, add the internal linear system to the problem's EquationSystems, and
   * assemble the operator.
   *
   * This runs on the first call to updatePseudoDisplacement() instead of in initialSetup(),
   * because it needs the reference coordinates that the engine snapshots at the very end of
   * FEProblemBase::initialSetup().
   */
  void setupSystem();

  /**
   * Sort the boundary nodes into the moving and the fixed Dirichlet sets, and rederive the default
   * fixed boundaries when the user did not name them.
   */
  void collectConstrainedNodes();

  /**
   * Assemble the Laplace operator on Omega_0 and replace the Dirichlet rows with identity rows.
   *
   * The nodes currently sit at x = X0 + d, so they are moved back to X0 for the duration of the
   * assembly and put back exactly where they were on the way out. Their columns are left in place,
   * which is what carries the prescribed values into the interior equations.
   *
   * The ghosted nodes are moved along with the local ones, so this requires every node this
   * processor holds to be keyed in X0 and in d. The engine guarantees that: it refreshes the
   * entries of the ghosted nodes at the start of every step, and it snapshots both maps over the
   * whole mesh right before it calls reinitOnNewMesh().
   */
  void assembleLaplacian();

  /**
   * The prescribed total pseudo-displacement of every constrained node this rank owns.
   *
   * @param dt the time step size the executioner is about to solve with
   * @param values keyed by degree of freedom, filled with d_prev + dt * v on the moving boundaries
   *               and with zero on the fixed ones
   */
  void computeBoundaryValues(Real dt, std::map<dof_id_type, libMesh::Point> & values);

  /**
   * The interface velocity component at a node of a moving boundary.
   *
   * @param node a node of a moving boundary that this rank owns
   * @param component the spatial component
   * @param t the time the prescribed velocity is evaluated at
   */
  Real interfaceVelocity(const libMesh::Node & node, unsigned int component, Real t) const;

  /**
   * The boundary ids carried by the element sides that have no neighbor, reduced over the
   * communicator because a rank only sees the sides of its own elements.
   */
  std::set<BoundaryID> externalBoundaryIds() const;

  /// The boundaries whose motion drives the smoothing
  std::set<BoundaryID> _moving_boundary_ids;

  /// The boundaries pinned at a zero pseudo-displacement
  std::set<BoundaryID> _fixed_boundary_ids;

  /// Whether the interface velocity is prescribed with functions rather than read from variables
  const bool _prescribed_velocity;

  /// The prescribed interface velocity, one function per spatial component. Only filled when
  /// _prescribed_velocity.
  std::vector<const Function *> _velocity_functions;

  /// The interface velocity variables, one per spatial component. Only filled when
  /// !_prescribed_velocity.
  std::vector<const MooseVariableFieldBase *> _velocity_variables;

  /// The nodes of the moving boundaries, keyed by node id
  std::map<dof_id_type, const libMesh::Node *> _moving_nodes;

  /// The nodes of the fixed boundaries that are not also on a moving boundary, keyed by node id
  std::map<dof_id_type, const libMesh::Node *> _fixed_nodes;

  /**
   * Which spatial components are pinned for each fixed node, keyed by node id. On a fixed wall
   * that shares a corner node with a moving boundary the pinned components come from the wall's
   * reference normals, so a flat axis-perpendicular wall pins only its normal component and its
   * nodes slide tangentially - which keeps the travelling corner from overtaking them - while an
   * oblique wall pins every component its normal points along. Every other fixed wall pins all
   * components, exactly as if it carried no sliding at all.
   */
  std::map<dof_id_type, std::array<bool, 3>> _fixed_node_components;

  /// The Laplacian without any constrained row, cloned in assembleLaplacian(). The identity rows
  /// differ by component once fixed walls pin only their normal component, so each component solve
  /// restores this base operator and applies its own rows.
  std::unique_ptr<libMesh::SparseMatrix<libMesh::Number>> _base_matrix;

  /// The matrix rows of the moving nodes this rank owns, pinned for every component
  std::vector<libMesh::numeric_index_type> _moving_rows;

  /// The matrix rows of the fixed nodes this rank owns, one set per pinned spatial component
  std::array<std::vector<libMesh::numeric_index_type>, 3> _fixed_rows;

  /// The internal linear system the pseudo-displacement is solved on, null until setupSystem()
  libMesh::LinearImplicitSystem * _sys;

  /// The number of the single variable of _sys
  unsigned int _var_num;
};
