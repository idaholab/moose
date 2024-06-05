//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesProblem.h"
#include "NonlinearSystemBase.h"
#include "libmesh/petsc_matrix.h"

registerMooseObject("NavierStokesApp", NavierStokesProblem);

InputParameters
NavierStokesProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addClassDescription("A problem that handles Schur complement preconditioning of the "
                             "incompressible Navier-Stokes equations");
  params.addParam<TagName>(
      "mass_matrix", "", "The matrix tag name corresponding to the mass matrix.");
  params.addParam<TagName>(
      "L_matrix",
      "",
      "The matrix tag name corresponding to the diffusive part of the velocity equations.");
  params.addParam<std::vector<unsigned int>>(
      "schur_fs_index",
      {},
      "if not provided then the top field split is assumed to be the "
      "Schur split. This is a vector to allow recursive nesting");
  params.addParam<bool>("use_pressure_mass_matrix",
                        false,
                        "Whether to just use the pressure mass matrix as the preconditioner for "
                        "the Schur complement");
  params.addParam<bool>(
      "commute_lsc",
      false,
      "Whether to use the commuted form of the LSC preconditioner, created by Olshanskii");
  return params;
}

NavierStokesProblem::NavierStokesProblem(const InputParameters & parameters) : FEProblem(parameters)
#if PETSC_RELEASE_LESS_THAN(3, 20, 0)
{
  mooseError("The preconditioning techniques made available through this class require a PETSc "
             "version of at least 3.20");
}
#else
    ,
    _commute_lsc(getParam<bool>("commute_lsc")),
    _mass_matrix(getParam<TagName>("mass_matrix")),
    _L_matrix(getParam<TagName>("L_matrix")),
    _have_mass_matrix(!_mass_matrix.empty()),
    _have_L_matrix(!_L_matrix.empty()),
    _pressure_mass_matrix_as_pre(getParam<bool>("use_pressure_mass_matrix")),
    _schur_fs_index(getParam<std::vector<unsigned int>>("schur_fs_index"))
{
  if (_commute_lsc)
  {
    if (!_have_mass_matrix)
      paramError("mass_matrix",
                 "A pressure mass matrix must be provided if we are commuting the LSC commutator.");
    if (!_have_L_matrix)
      paramError("L_matrix",
                 "A matrix corresponding to the viscous component of the momentum equation must be "
                 "provided if we are commuting the LSC commutator.");
  }
  else if (_have_L_matrix)
    paramError("L_matrix",
               "If not commuting the LSC commutator, then the 'L_matrix' should not be provided "
               "because it will not be used. For Elman LSC preconditioning, L will be computed "
               "automatically using system matrix data (e.g. the off-diagonal blocks in the "
               "velocity-pressure system).");

  if (_pressure_mass_matrix_as_pre && !_have_mass_matrix)
    paramError("mass_matrix",
               "If 'use_pressure_mass_matrix', then a pressure 'mass_matrix' must be provided");
}

NavierStokesProblem::~NavierStokesProblem()
{
  auto ierr = (PetscErrorCode)0;
  if (_Q_scale)
  {
    ierr = MatDestroy(&_Q_scale);
    CHKERRABORT(this->comm().get(), ierr);
  }
  if (_L)
  {
    ierr = MatDestroy(&_L);
    CHKERRABORT(this->comm().get(), ierr);
  }
}

KSP
NavierStokesProblem::findSchurKSP(KSP node, const unsigned int tree_position)
{
  auto it = _schur_fs_index.begin() + tree_position;
  if (it == _schur_fs_index.end())
    return node;

  auto ierr = (PetscErrorCode)0;
  PC fs_pc;
  PetscInt num_splits;
  KSP * subksp;
  KSP next_ksp;
  IS is;
  PetscBool is_fs;

  auto sub_ksp_index = *it;

  // Get the preconditioner associated with the linear solver
  ierr = KSPGetPC(node, &fs_pc);
  LIBMESH_CHKERR2(this->comm(), ierr);

  // Verify the preconditioner is a field split preconditioner
  ierr = PetscObjectTypeCompare((PetscObject)fs_pc, PCFIELDSPLIT, &is_fs);
  LIBMESH_CHKERR2(this->comm(), ierr);
  if (!is_fs)
    mooseError("Not a field split. Please check the 'schur_fs_index' parameter");

  // Setup the preconditioner. We need to call this first in order to be able to retrieve the sub
  // ksps and sub index sets associated with the splits
  ierr = PCSetUp(fs_pc);
  LIBMESH_CHKERR2(this->comm(), ierr);

  // Get the linear solvers associated with each split
  ierr = PCFieldSplitGetSubKSP(fs_pc, &num_splits, &subksp);
  LIBMESH_CHKERR2(this->comm(), ierr);
  next_ksp = subksp[sub_ksp_index];

  // Get the index set for the split at this level of the tree we are traversing to the Schur
  // complement preconditioner
  ierr = PCFieldSplitGetISByIndex(fs_pc, sub_ksp_index, &is);
  LIBMESH_CHKERR2(this->comm(), ierr);

  // Store this tree level's index set, which we will eventually use to get the sub-matrices
  // required for our preconditioning process from the system matrix
  _index_sets.push_back(is);

  // Free the array of sub linear solvers that got allocated in the PCFieldSplitGetSubKSP call
  ierr = PetscFree(subksp);
  LIBMESH_CHKERR2(this->comm(), ierr);

  // Continue traversing down the tree towards the Schur complement linear solver/preconditioner
  return findSchurKSP(next_ksp, tree_position + 1);
}

void
NavierStokesProblem::setupLSCMatrices(KSP schur_ksp)
{
  KSP * subksp; // This will be length two, with the former being the A KSP and the latter being the
                // Schur complement KSP
  KSP schur_complement_ksp;
  PC schur_pc, lsc_pc;
  PetscInt num_splits;
  Mat lsc_pc_pmat;
  IS velocity_is, pressure_is;
  PetscInt rstart, rend;
  PetscBool is_lsc, is_fs;
  std::vector<Mat> intermediate_Qs;
  std::vector<Mat> intermediate_Ls;
  auto ierr = (PetscErrorCode)0;

  // Get the preconditioner for the linear solver. It must be a field split preconditioner
  ierr = KSPGetPC(schur_ksp, &schur_pc);
  LIBMESH_CHKERR2(this->comm(), ierr);
  ierr = PetscObjectTypeCompare((PetscObject)schur_pc, PCFIELDSPLIT, &is_fs);
  LIBMESH_CHKERR2(this->comm(), ierr);
  if (!is_fs)
    mooseError("Not a field split. Please check the 'schur_fs_index' parameter");

  // The mass matrix. This will correspond to velocity degrees of freedom for Elman LSC and pressure
  // degrees of freedom for Olshanskii LSC or when directly using the mass matrix as a
  // preconditioner for the Schur complement
  Mat global_Q = nullptr;
  if (_have_mass_matrix)
    global_Q =
        static_cast<PetscMatrix<Number> &>(getNonlinearSystemBase(0).getMatrix(massMatrixTagID()))
            .mat();
  // The Poisson operator matrix corresponding to the velocity degrees of freedom. This is only used
  // and is required for Olshanskii LSC preconditioning
  Mat global_L = nullptr;
  if (_have_L_matrix)
    global_L =
        static_cast<PetscMatrix<Number> &>(getNonlinearSystemBase(0).getMatrix(LMatrixTagID()))
            .mat();

  //
  // Process down from our system matrix to the sub-matrix containing the velocity-pressure dofs for
  // which we are going to be doing the Schur complement preconditioning
  //

  auto process_intermediate_mats = [this, &ierr](auto & intermediate_mats, auto parent_mat)
  {
    mooseAssert(parent_mat, "This should be non-null");
    intermediate_mats.resize(_index_sets.size());
    for (const auto i : index_range(_index_sets))
    {
      auto intermediate_is = _index_sets[i];
      Mat intermediate_mat;
      ierr = MatCreateSubMatrix(i == 0 ? parent_mat : intermediate_mats[i - 1],
                                intermediate_is,
                                intermediate_is,
                                MAT_INITIAL_MATRIX,
                                &intermediate_mat);
      LIBMESH_CHKERR2(this->comm(), ierr);
      intermediate_mats[i] = intermediate_mat;
    }
    return _index_sets.empty() ? parent_mat : intermediate_mats.back();
  };

  Mat our_parent_Q = nullptr;
  if (_have_mass_matrix)
    our_parent_Q = process_intermediate_mats(intermediate_Qs, global_Q);
  Mat our_parent_L = nullptr;
  if (_have_L_matrix)
    our_parent_L = process_intermediate_mats(intermediate_Ls, global_L);

  // Setup the preconditioner. We need to call this first in order to be able to retrieve the sub
  // ksps and sub index sets associated with the splits
  ierr = PCSetUp(schur_pc);
  LIBMESH_CHKERR2(this->comm(), ierr);

  // There are always two splits in a Schur complement split. The zeroth split is the split with the
  // on-diagonals, e.g. the velocity dofs. Here we retrive the velocity dofs/index set
  ierr = PCFieldSplitGetISByIndex(schur_pc, 0, &velocity_is);
  LIBMESH_CHKERR2(this->comm(), ierr);

  // Get the rows of the parent velocity-pressure matrix that our process owns
  ierr = MatGetOwnershipRange(our_parent_Q, &rstart, &rend);
  LIBMESH_CHKERR2(this->comm(), ierr);

  if (_commute_lsc)
  {
    // If we're commuting LSC, e.g. doing Olshanskii, the user must have provided a Poisson operator
    // matrix
    mooseAssert(our_parent_L, "This should be non-null");
    if (!_L)
    {
      // If this is our first time in this routine, then we create the matrix
      ierr = MatCreateSubMatrix(our_parent_L, velocity_is, velocity_is, MAT_INITIAL_MATRIX, &_L);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
    else
    {
      // Else we reuse the matrix
      ierr = MatCreateSubMatrix(our_parent_L, velocity_is, velocity_is, MAT_REUSE_MATRIX, &_L);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
  }

  // Get the local index set complement corresponding to the pressure dofs from the velocity dofs
  ierr = ISComplement(velocity_is, rstart, rend, &pressure_is);
  LIBMESH_CHKERR2(this->comm(), ierr);

  auto create_q_scale_submat =
      [our_parent_Q, this, velocity_is, pressure_is, &ierr](const auto & mat_initialization)
  {
    if (_commute_lsc || _pressure_mass_matrix_as_pre)
    {
      // If we are doing Olshanskii or we are using the pressure matrix directly as the
      // preconditioner (no LSC), then we must have access to a pressure mass matrix
      mooseAssert(our_parent_Q, "This should be non-null");
      // Create a sub-matrix corresponding to the pressure index set
      ierr =
          MatCreateSubMatrix(our_parent_Q, pressure_is, pressure_is, mat_initialization, &_Q_scale);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
    else if (_have_mass_matrix) // If we don't have a mass matrix and the user has requested scaling
                                // then the diagonal of A will be used
    {
      // The user passed us a mass matrix tag; we better have been able to obtain a parent Q in that
      // case
      mooseAssert(our_parent_Q, "This should be non-null");
      // We are not commuting LSC, so we are doing Elman, and the user has passed us a mass matrix
      // tag. In this case we are creating a velocity mass matrix, so we use the velocity index set
      ierr =
          MatCreateSubMatrix(our_parent_Q, velocity_is, velocity_is, mat_initialization, &_Q_scale);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
  };

  if (!_Q_scale)
    // We haven't allocated the scaling matrix yet
    create_q_scale_submat(MAT_INITIAL_MATRIX);
  else
    // We have allocated the scaling matrix, so we can reuse
    create_q_scale_submat(MAT_REUSE_MATRIX);

  // We don't need the pressure index set anymore
  ierr = ISDestroy(&pressure_is);
  LIBMESH_CHKERR2(this->comm(), ierr);

  // Nor the intermediate matrices
  for (auto & mat : intermediate_Qs)
  {
    ierr = MatDestroy(&mat);
    LIBMESH_CHKERR2(this->comm(), ierr);
  }
  for (auto & mat : intermediate_Ls)
  {
    ierr = MatDestroy(&mat);
    LIBMESH_CHKERR2(this->comm(), ierr);
  }

  // Get the sub KSP for the Schur split that corresponds to the linear solver for the Schur
  // complement (e.g. rank equivalent to the pressure rank)
  ierr = PCFieldSplitGetSubKSP(schur_pc, &num_splits, &subksp);
  LIBMESH_CHKERR2(this->comm(), ierr);
  if (num_splits != 2)
    mooseError("The number of splits should be two");
  // The Schur complement linear solver is always at the first index (for the pressure dofs;
  // velocity dof KSP is at index 0)
  schur_complement_ksp = subksp[1];

  if (_pressure_mass_matrix_as_pre)
  {
    mooseAssert(_Q_scale, "This should be non-null");
    Mat S;
    // Set the Schur complement preconditioner to be the pressure mass matrix
    ierr = PCFieldSplitSetSchurPre(schur_pc, PC_FIELDSPLIT_SCHUR_PRE_USER, _Q_scale);
    LIBMESH_CHKERR2(this->comm(), ierr);
    // Get the Schur complement operator S, which in generic KSP speak is used for the operator A
    ierr = KSPGetOperators(schur_complement_ksp, &S, NULL);
    LIBMESH_CHKERR2(this->comm(), ierr);
    // Set, in generic KSP speak, the operators A and P respectively. So our pressure mass matrix is
    // P
    ierr = KSPSetOperators(schur_complement_ksp, S, _Q_scale);
    LIBMESH_CHKERR2(this->comm(), ierr);
  }
  else // We are doing LSC preconditioning
  {
    // Get the least squares commutator preconditioner for the Schur complement
    ierr = KSPGetPC(schur_complement_ksp, &lsc_pc);
    LIBMESH_CHKERR2(this->comm(), ierr);
    // Verify that it's indeed an LSC preconditioner
    ierr = PetscObjectTypeCompare(PetscObject(lsc_pc), PCLSC, &is_lsc);
    LIBMESH_CHKERR2(this->comm(), ierr);
    if (!is_lsc)
      mooseError("Not an LSC PC. Please check the 'schur_fs_index' parameter");

    // Get the LSC preconditioner
    ierr = PCGetOperators(lsc_pc, NULL, &lsc_pc_pmat);
    LIBMESH_CHKERR2(this->comm(), ierr);

    if (_commute_lsc)
    {
      // We're doing Olshanskii. We must have a user-provided Poisson operator
      mooseAssert(_L, "This should be non-null");
      // Attach our L matrix to the PETSc object. PETSc will use this during the preconditioner
      // application
      ierr = PetscObjectCompose((PetscObject)lsc_pc_pmat, "LSC_L", (PetscObject)_L);
      LIBMESH_CHKERR2(this->comm(), ierr);
      // Olshanskii preconditioning requires a pressure mass matrix
      mooseAssert(_have_mass_matrix, "This is to verify we will enter the next conditional");
    }
    if (_have_mass_matrix)
    {
      mooseAssert(_Q_scale, "This should be non-null");
      // Attach our scaling/mass matrix to the PETSc object. PETSc will use this during the
      // preconditioner application
      ierr = PetscObjectCompose((PetscObject)lsc_pc_pmat, "LSC_Qscale", (PetscObject)_Q_scale);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
  }

  // Free the sub-KSP array that was allocated during PCFieldSplitGetSubKSP
  ierr = PetscFree(subksp);
  LIBMESH_CHKERR2(this->comm(), ierr);
}

PetscErrorCode
navierStokesKSPPreSolve(KSP root_ksp, Vec /*rhs*/, Vec /*x*/, void * context)
{
  PetscFunctionBegin;

  auto * ns_problem = static_cast<NavierStokesProblem *>(context);
  ns_problem->clearIndexSets();
  auto schur_ksp = ns_problem->findSchurKSP(root_ksp, 0);
  ns_problem->setupLSCMatrices(schur_ksp);

  PetscFunctionReturn(PETSC_SUCCESS);
}

void
NavierStokesProblem::initPetscOutputAndSomeSolverSettings()
{
  FEProblem::initPetscOutputAndSomeSolverSettings();

  if (!_have_mass_matrix)
  {
    mooseAssert(
        !_have_L_matrix,
        "If we don't have a mass matrix, which is only supported for traditional LSC "
        "preconditioning (e.g. Elman, not Olshanskii), then we also shouldn't have an L matrix "
        "because we automatically form the L matrix when doing traditional LSC preconditioning");
    return;
  }

  // Set the pre-KSP solve callback. At that time we will setup our Schur complement preconditioning
  auto ierr = (PetscErrorCode)0;
  KSP ksp;
  auto snes = currentNonlinearSystem().getSNES();
  ierr = SNESGetKSP(snes, &ksp);
  LIBMESH_CHKERR2(this->comm(), ierr);
  ierr = KSPSetPreSolve(ksp, &navierStokesKSPPreSolve, this);
  LIBMESH_CHKERR2(this->comm(), ierr);
}

#endif
