//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblem.h"
#include "libmesh/libmesh_config.h"
#include <petscsnes.h>

class NonlinearSystem;

/**
 * A problem that handles Schur complement preconditioning of the incompressible Navier-Stokes
 * equations
 */
class NavierStokesProblem : public FEProblem
{
public:
  static InputParameters validParams();

  NavierStokesProblem(const InputParameters & parameters);

#if PETSC_RELEASE_GREATER_EQUALS(3, 20, 0)
  /**
   * @returns the mass matrix tag ID
   */
  TagID massMatrixTagID() const { return getMatrixTagID(_mass_matrix); }

  /**
   * @returns the poisson operator matrix tag ID
   */
  TagID LMatrixTagID() const { return getMatrixTagID(_L_matrix); }

  /**
   * Clear the field split index sets
   */
  void clearIndexSets() { _index_sets.clear(); }

  /*
   * Given a \p KSP \p node and where we are in the field split tree, given by \p tree_position,
   * return the next \p KSP object in the tree on the way to the Schur complement \p KSP object.
   * Each invocation of this method moves through one level of our \p _index_sets data member. This
   * method will call itself recursively until it reaches the Schur complement \p KSP
   */
  KSP findSchurKSP(KSP node, unsigned int tree_position);

  /**
   * Setup the Least Squares Commutator (LSC) preconditioner given the Schur complement \p KSP
   * object
   */
  void setupLSCMatrices(KSP schur_ksp);

  /**
   * Will destroy any matrices we allocated
   */
  virtual ~NavierStokesProblem();

protected:
  /**
   * Reinitialize PETSc output for proper linear/nonlinear iteration display
   */
  virtual void initPetscOutputAndSomeSolverSettings() override;

private:
  /// Whether to commute operators in the style of Olshanskii. If this is true, then the user must
  /// provide both (pressure) mass matrices and a Poisson operator for the velocity
  const bool _commute_lsc;
  /// The tag name of the mass matrix
  const TagName & _mass_matrix;
  /// The tag name of the Poisson operator
  const TagName & _L_matrix;
  /// Whether the user attached a mass matrix
  const bool _have_mass_matrix;
  /// Whether the user attached a Poisson operator matrix
  const bool _have_L_matrix;

  /// Whether to directly use the pressure mass matrix to form the Schur complement
  /// preconditioner. This is only appropriate for Stokes flow in which the pressure mass matrix is
  /// spectrally equivalent to the Schur complement
  const bool _pressure_mass_matrix_as_pre;

  /// The length of this vector should correspond to the number of split nesting levels there are in
  /// the field split. Then the integers should indicate the path one shold take in the nesting tree
  /// to get to the location of the Schur complement field split
  const std::vector<unsigned int> & _schur_fs_index;

  /// The mass matrix used for scaling
  Mat _Q_scale = nullptr;
  /// The Poisson operator
  Mat _L = nullptr;

  /// This will end up being the same length as \p _schur_fs_index. Let's give an example of what
  /// this data member means. If the user sets "schur_fs_index = '1'", then this means the Schur
  /// complement field split is nested within another field split, and the Schur complement field
  /// split is at the 1st index of the top split (some other set of degrees of freedom take up the
  /// 0th index of the top split). So in this example \p _index_sets will be of length 1, and the
  /// Index Set (IS) held by this container will hold all the Schur complement field split degrees
  /// of freedom (e.g. all the system degrees of freedom minus the degrees of freedom held in the
  /// 0th index of the top split). An example of this example is if we split out all the velocity
  /// Dirichlet degrees of freedom into the 0th index of the top split, and then our Schur
  /// complement at index 1 of the top split handles all non-Dirichlet velocity degrees of freedom
  /// and all pressure degrees of freedom
  std::vector<IS> _index_sets;
#endif
};
