//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
   * Will destroy any matrices we allocated
   */
  virtual ~NavierStokesProblem();

  virtual void initialSetup() override;

protected:
  /**
   * Reinitialize PETSc output for proper linear/nonlinear iteration display
   */
  virtual void initPetscOutputAndSomeSolverSettings() override;

private:
  /**
   * Context object attached to a field split PC via PCSetApplicationContext so that the static
   * \p fieldSplitPostSetUpCallback can recover which problem and which position in the field
   * split tree triggered the callback
   */
  struct FieldSplitPostSetUpContext
  {
    /// The problem that owns the field split tree being set up
    NavierStokesProblem * problem;
    /// The position of this PC within the field split tree; see \p _schur_fs_index
    std::size_t tree_position;
  };

  /// Run after PETSc sets up a field split in the tree leading to the Schur complement
  static PetscErrorCode fieldSplitPostSetUpCallback(PC pc);

  /// Continue through the field split tree or set up the target Schur complement matrices
  void fieldSplitPostSetUp(PC pc, std::size_t tree_position);

  /// Install the post-setup callback and its context on a field split PC
  void setFieldSplitPostSetUp(PC pc, std::size_t tree_position);

  /// Set up the Least Squares Commutator (LSC) preconditioner for the Schur complement
  void setupLSCMatrices(PC schur_pc);

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

  /// Whether and what to set as the user-provided Schur complement preconditioner MASS is only
  /// appropriate for Stokes flow or augmented Lagrange formulations in which the pressure mass
  /// matrix is spectrally equivalent to the Schur complement
  const enum class SetSchurPreType { FALSE, MASS, A11_AND_MASS } _set_schur_pre;

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

  /// Stable callback contexts for each level of the field split tree and its terminal node
  std::vector<FieldSplitPostSetUpContext> _field_split_post_setup_contexts;
#endif
};
