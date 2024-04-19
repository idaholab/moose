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
  params.addParam<bool>(
      "use_composite_for_A",
      false,
      "Whether to use a composite preconditioner for the velocity (momentum) block");
  params.addParam<Real>("alpha",
                        1,
                        "A coefficient multipliying the identity matrix that will be added to the "
                        "individual matrices for composite preconditioning of the velocity block");
  params.addParam<TagName>(
      "A_matrix", "", "The advection-diffusion operator for the velocity block");
  params.addParam<TagName>("J_matrix", "", "The singular perturbation to the velocity block");
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
    _A_matrix(getParam<TagName>("A_matrix")),
    _J_matrix(getParam<TagName>("J_matrix")),
    _have_mass_matrix(!_mass_matrix.empty()),
    _have_L_matrix(!_L_matrix.empty()),
    _have_A_matrix(!_A_matrix.empty()),
    _have_J_matrix(!_J_matrix.empty()),
    _pressure_mass_matrix_as_pre(getParam<bool>("use_pressure_mass_matrix")),
    _schur_fs_index(getParam<std::vector<unsigned int>>("schur_fs_index")),
    _use_composite_for_A(getParam<bool>("use_composite_for_A")),
    _alpha(getParam<Real>("alpha"))
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
    paramError("mass_matrix", "If 'use_pressure_mass_matrix', then a mass_matrix must be provided");

  if (_use_composite_for_A)
  {
    if (!_pressure_mass_matrix_as_pre)
      paramError(
          "use_composite_for_A",
          "Conceptually we only support composite preconditioning for A with an augmented Lagrange "
          "strategy. With an augmented Lagrange strategy, the goal is to make the Schur complement "
          "spectrally equivalent to the pressure mass matrix. Consequently if "
          "'use_composite_for_A' "
          "is true, then 'pressure_mass_matrix_as_pre' must be supplied and be true.");

    if (!_have_A_matrix || !_have_J_matrix)
      paramError("use_composite_for_A",
                 "Composite preconditioning for A requires that both an 'A_matrix' and 'J_matrix' "
                 "tag be provided");
  }
  else if (isParamSetByUser("alpha"))
    paramError("alpha", "This is only used if 'use_composite_for_A' is supplied and set to true");
}

NavierStokesProblem::~NavierStokesProblem()
{
  auto destroy_mat = [](Mat mat)
  {
    if (mat)
      MatDestroy(&mat);
  };

  destroy_mat(_M);
  destroy_mat(_L);
  destroy_mat(_A);
  destroy_mat(_J);
  destroy_mat(_AplusD);
  destroy_mat(_JplusD);
  destroy_mat(_D);
}

KSP
NavierStokesProblem::findSchurKSP(KSP node, const unsigned int tree_position)
{
  auto it = _schur_fs_index.begin() + tree_position;
  if (it == _schur_fs_index.end())
    return node;

  PetscErrorCode ierr = 0;
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
  PetscObjectTypeCompare((PetscObject)fs_pc, PCFIELDSPLIT, &is_fs);
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
NavierStokesProblem::setupMatrices(KSP schur_ksp)
{
  KSP * subksp; // This will be length two, with the former being the A KSP and the latter being the
                // Schur complement KSP
  KSP schur_complement_ksp, inner_velocity_ksp;
  PC schur_pc, lsc_pc;
  PetscInt num_splits;
  Mat lsc_pc_pmat;
  IS velocity_is, pressure_is;
  PetscInt rstart, rend;
  PetscBool is_lsc, is_fs;
  std::vector<Mat> intermediate_Ms;
  std::vector<Mat> intermediate_Ls;
  std::vector<Mat> intermediate_As;
  std::vector<Mat> intermediate_Js;
  PetscErrorCode ierr = 0;

  // Get the preconditioner for the linear solver. It must be a field split preconditioner
  ierr = KSPGetPC(schur_ksp, &schur_pc);
  LIBMESH_CHKERR2(this->comm(), ierr);
  PetscObjectTypeCompare((PetscObject)schur_pc, PCFIELDSPLIT, &is_fs);
  LIBMESH_CHKERR2(this->comm(), ierr);
  if (!is_fs)
    mooseError("Not a field split. Please check the 'schur_fs_index' parameter");

  // The mass matrix. This will correspond to velocity degrees of freedom for Elman LSC and pressure
  // degrees of freedom for Olshanskii LSC or when directly using the mass matrix as a
  // preconditioner for the Schur complement
  Mat global_M = nullptr;
  if (_have_mass_matrix)
    global_M =
        static_cast<PetscMatrix<Number> &>(getNonlinearSystemBase(0).getMatrix(massMatrixTagID()))
            .mat();
  // The Poisson operator matrix corresponding to the velocity degrees of freedom. This is only used
  // and is required for Olshanskii LSC preconditioning
  Mat global_L = nullptr;
  if (_have_L_matrix)
    global_L =
        static_cast<PetscMatrix<Number> &>(getNonlinearSystemBase(0).getMatrix(LMatrixTagID()))
            .mat();

  Mat global_A = nullptr;
  if (_have_A_matrix)
    global_A = static_cast<PetscMatrix<Number> &>(
                   getNonlinearSystemBase(0).getMatrix(getMatrixTagID(_A_matrix)))
                   .mat();
  Mat global_J = nullptr;
  if (_have_J_matrix)
    global_J = static_cast<PetscMatrix<Number> &>(
                   getNonlinearSystemBase(0).getMatrix(getMatrixTagID(_J_matrix)))
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

  Mat our_parent_M = nullptr;
  if (_have_mass_matrix)
    our_parent_M = process_intermediate_mats(intermediate_Ms, global_M);
  Mat our_parent_L = nullptr;
  if (_have_L_matrix)
    our_parent_L = process_intermediate_mats(intermediate_Ls, global_L);
  Mat our_parent_A = nullptr;
  if (_have_A_matrix)
    our_parent_A = process_intermediate_mats(intermediate_As, global_A);
  Mat our_parent_J = nullptr;
  if (_have_J_matrix)
    our_parent_J = process_intermediate_mats(intermediate_Js, global_J);

  // Setup the preconditioner. We need to call this first in order to be able to retrieve the sub
  // ksps and sub index sets associated with the splits
  ierr = PCSetUp(schur_pc);
  LIBMESH_CHKERR2(this->comm(), ierr);

  // There are always two splits in a Schur complement split. The zeroth split is the split with the
  // on-diagonals, e.g. the velocity dofs. Here we retrive the velocity dofs/index set
  ierr = PCFieldSplitGetISByIndex(schur_pc, 0, &velocity_is);
  LIBMESH_CHKERR2(this->comm(), ierr);

  // Get the rows of the parent velocity-pressure matrix that our process owns
  ierr = MatGetOwnershipRange(our_parent_M, &rstart, &rend);
  LIBMESH_CHKERR2(this->comm(), ierr);

  //
  // Create our needed submatrices
  //

  auto create_submatrix = [&ierr, this](Mat parent_mat, IS is, Mat & child_mat)
  {
    mooseAssert(parent_mat, "This should be non-null");
    if (!child_mat)
    {
      // If this is our first time in this routine, then we create the matrix
      ierr = MatCreateSubMatrix(parent_mat, is, is, MAT_INITIAL_MATRIX, &child_mat);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
    else
    {
      // Else we reuse the matrix
      ierr = MatCreateSubMatrix(parent_mat, is, is, MAT_REUSE_MATRIX, &child_mat);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
  };

  if (_commute_lsc)
    create_submatrix(our_parent_L, velocity_is, _L);

  // Get the local index set complement corresponding to the pressure dofs from the velocity dofs
  ierr = ISComplement(velocity_is, rstart, rend, &pressure_is);
  LIBMESH_CHKERR2(this->comm(), ierr);

  if (_commute_lsc || _pressure_mass_matrix_as_pre)
    create_submatrix(our_parent_M, pressure_is, _M);
  else
    create_submatrix(our_parent_M, velocity_is, _M);

  if (_use_composite_for_A)
  {
    create_submatrix(our_parent_A, velocity_is, _A);
    create_submatrix(our_parent_J, velocity_is, _J);
  }

  //
  // Destroy submatrix precursor data we no longer need
  //

  // We don't need the pressure index set anymore
  ierr = ISDestroy(&pressure_is);
  LIBMESH_CHKERR2(this->comm(), ierr);

  auto destroy_intermediate_mats = [&ierr, this](auto & intermediate_mats)
  {
    for (auto & mat : intermediate_mats)
    {
      ierr = MatDestroy(&mat);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
  };

  destroy_intermediate_mats(intermediate_Ms);
  destroy_intermediate_mats(intermediate_Ls);
  destroy_intermediate_mats(intermediate_As);
  destroy_intermediate_mats(intermediate_Js);

  //
  // Now setup the field split preconditioners armed with our data
  //

  // Get the sub KSP for the Schur split that corresponds to the linear solver for the Schur
  // complement (e.g. rank equivalent to the pressure rank)
  ierr = PCFieldSplitGetSubKSP(schur_pc, &num_splits, &subksp);
  LIBMESH_CHKERR2(this->comm(), ierr);
  if (num_splits != 2)
    mooseError("The number of splits should be two");
  // The Schur complement linear solver is always at the first index (for the pressure dofs;
  // velocity dof KSP is at index 0)
  schur_complement_ksp = subksp[1];
  inner_velocity_ksp = subksp[0];

  if (_pressure_mass_matrix_as_pre)
  {
    mooseAssert(_M, "This should be non-null");
    Mat S;
    // Set the Schur complement preconditioner to be the pressure mass matrix
    ierr = PCFieldSplitSetSchurPre(schur_pc, PC_FIELDSPLIT_SCHUR_PRE_USER, _M);
    LIBMESH_CHKERR2(this->comm(), ierr);
    // Get the Schur complement operator S, which in generic KSP speak is used for the operator A
    ierr = KSPGetOperators(schur_complement_ksp, &S, NULL);
    LIBMESH_CHKERR2(this->comm(), ierr);
    // Set, in generic KSP speak, the operators A and P respectively. So our pressure mass matrix is
    // P
    ierr = KSPSetOperators(schur_complement_ksp, S, _M);
    LIBMESH_CHKERR2(this->comm(), ierr);

    if (_use_composite_for_A)
    {
      Mat AplusJ;
      Vec Diag;
      PetscInt i_local_begin, i_local_end, M, m;
      MatType mat_type;

      auto copy_mat = [&ierr, this](Mat mat_to_copy, Mat & mat_copy)
      {
        if (!mat_copy)
        {
          ierr = MatDuplicate(mat_to_copy, MAT_COPY_VALUES, &mat_copy);
          LIBMESH_CHKERR2(this->comm(), ierr);
        }
        else
        {
          ierr = MatCopy(mat_to_copy, mat_copy, SAME_NONZERO_PATTERN);
          LIBMESH_CHKERR2(this->comm(), ierr);
        }
      };
      copy_mat(_A, _AplusD);
      copy_mat(_J, _JplusD);

      // Create the diagonal matrix
      if (!_D)
      {
        ierr = MatGetType(_A, &mat_type);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr = MatGetSize(_A, &M, nullptr);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr = MatGetLocalSize(_A, &m, nullptr);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr = MatCreate(this->comm().get(), &_D);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr = MatSetSizes(_D, m, m, M, M);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr = MatSetType(_D, mat_type);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr = MatSetFromOptions(_D);
        LIBMESH_CHKERR2(this->comm(), ierr);
        // Setting the diagonal can be very slow if the matrix being modified doesn't have the
        // diagonal filled. So we fill with an identity here
        ierr = MatGetOwnershipRange(_D, &i_local_begin, &i_local_end);
        LIBMESH_CHKERR2(this->comm(), ierr);
        for (const auto i : make_range(i_local_begin, i_local_end))
        {
          static constexpr PetscScalar one = 1;
          ierr = MatSetValues(_D, 1, &i, 1, &i, &one, INSERT_VALUES);
          LIBMESH_CHKERR2(this->comm(), ierr);
        }
        ierr = MatAssemblyBegin(_D, MAT_FINAL_ASSEMBLY);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr = MatAssemblyEnd(_D, MAT_FINAL_ASSEMBLY);
        LIBMESH_CHKERR2(this->comm(), ierr);
      }
      ierr = MatCreateVecs(_D, &Diag, nullptr);
      LIBMESH_CHKERR2(this->comm(), ierr);
      ierr = MatSchurComplementGetSubMatrices(S, &AplusJ, nullptr, nullptr, nullptr, nullptr);
      LIBMESH_CHKERR2(this->comm(), ierr);
      ierr = MatGetDiagonal(AplusJ, Diag);
      LIBMESH_CHKERR2(this->comm(), ierr);
      ierr = MatDiagonalSet(_D, Diag, INSERT_VALUES);
      LIBMESH_CHKERR2(this->comm(), ierr);

      ierr = MatAXPY(_AplusD, _alpha, _D, SUBSET_NONZERO_PATTERN);
      LIBMESH_CHKERR2(this->comm(), ierr);
      // The I duplicated from A and J may have different nonzero patterns
      ierr = MatAXPY(_JplusD, _alpha, _D, SUBSET_NONZERO_PATTERN);
      LIBMESH_CHKERR2(this->comm(), ierr);

      {
        PetscViewer mat_viewer;
        PetscViewerBinaryOpen(PETSC_COMM_WORLD, "AplusJ", FILE_MODE_WRITE, &mat_viewer);
        MatView(AplusJ, mat_viewer);
        PetscViewerDestroy(&mat_viewer);
      }
      {
        PetscViewer mat_viewer;
        PetscViewerBinaryOpen(PETSC_COMM_WORLD, "AplusD", FILE_MODE_WRITE, &mat_viewer);
        MatView(_AplusD, mat_viewer);
        PetscViewerDestroy(&mat_viewer);
      }
      {
        PetscViewer mat_viewer;
        PetscViewerBinaryOpen(PETSC_COMM_WORLD, "JplusD", FILE_MODE_WRITE, &mat_viewer);
        MatView(_JplusD, mat_viewer);
        PetscViewerDestroy(&mat_viewer);
      }
      {
        PetscViewer mat_viewer;
        PetscViewerBinaryOpen(PETSC_COMM_WORLD, "D", FILE_MODE_WRITE, &mat_viewer);
        MatView(_D, mat_viewer);
        PetscViewerDestroy(&mat_viewer);
      }

      ierr = VecDestroy(&Diag);
      LIBMESH_CHKERR2(this->comm(), ierr);

      auto set_operators = [this, &ierr](KSP composite_ksp)
      {
        PCCompositeType comp_type;
        PC composite_pc;
        PC pc_A, pc_J;
        PetscBool composite_ksp_is_fgmres, is_composite, pc_A_is_ksp, pc_J_is_ksp;

        ierr = KSPGetPC(composite_ksp, &composite_pc);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr = PetscObjectTypeCompare(PetscObject(composite_pc), PCCOMPOSITE, &is_composite);
        LIBMESH_CHKERR2(this->comm(), ierr);
        if (!is_composite)
          mooseError("You specified to use a composite preconditioner for A, but this is not "
                     "specified by the PETSc options");
        ierr = PCCompositeGetType(composite_pc, &comp_type);
        LIBMESH_CHKERR2(this->comm(), ierr);
        if (comp_type != PC_COMPOSITE_SPECIAL)
          mooseError("The composite type must be special");
        ierr = PCCompositeGetPC(composite_pc, 0, &pc_A);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr = PCCompositeGetPC(composite_pc, 1, &pc_J);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr = PetscObjectTypeCompare(PetscObject(pc_A), PCKSP, &pc_A_is_ksp);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr = PetscObjectTypeCompare(PetscObject(pc_J), PCKSP, &pc_J_is_ksp);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr =
            PetscObjectTypeCompare(PetscObject(composite_ksp), KSPFGMRES, &composite_ksp_is_fgmres);
        LIBMESH_CHKERR2(this->comm(), ierr);
        if ((pc_A_is_ksp || pc_J_is_ksp) && !composite_ksp_is_fgmres)
          mooseError("If either of the composite preconditioners are KSP, then the outer composite "
                     "KSP must be FGMRES");

        ierr = PCSetOperators(pc_A, _AplusD, _AplusD);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr = PCSetOperators(pc_J, _JplusD, _JplusD);
        LIBMESH_CHKERR2(this->comm(), ierr);
        ierr = PCCompositeSpecialSetAlphaMat(composite_pc, _D);
        LIBMESH_CHKERR2(this->comm(), ierr);

        auto set_sub_pc_operators = [this, &ierr](PC ksp_pc, Mat mat)
        {
          KSP ksp;
          PC sub_pc;
          ierr = PCKSPGetKSP(ksp_pc, &ksp);
          LIBMESH_CHKERR2(this->comm(), ierr);
          ierr = KSPGetPC(ksp, &sub_pc);
          LIBMESH_CHKERR2(this->comm(), ierr);
          ierr = PCSetOperators(sub_pc, mat, mat);
          LIBMESH_CHKERR2(this->comm(), ierr);
          ierr = KSPSetFromOptions(ksp);
          LIBMESH_CHKERR2(this->comm(), ierr);
          ierr = PCSetFromOptions(sub_pc);
          LIBMESH_CHKERR2(this->comm(), ierr);
        };

        if (pc_A_is_ksp)
          set_sub_pc_operators(pc_A, _AplusD);
        if (pc_J_is_ksp)
          set_sub_pc_operators(pc_J, _JplusD);
      };

      PetscInt n_splits;
      KSP * schur_subksp;
      KSP ksp_outerA;
      ierr = PCFieldSplitSchurGetSubKSP(schur_pc, &n_splits, &schur_subksp);
      LIBMESH_CHKERR2(this->comm(), ierr);
      if (n_splits != 2)
        mooseError("Make sure that an 'inner' prefix is used so that a composite Schur is made "
                   "which allows different PC/KSP for outer and inner A solves");
      ksp_outerA = schur_subksp[0];
      libmesh_assert(ksp_outerA != inner_velocity_ksp);
      libmesh_assert(schur_complement_ksp == schur_subksp[1]);
      set_operators(inner_velocity_ksp);
      set_operators(ksp_outerA);

      ierr = PetscFree(schur_subksp);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
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
      mooseAssert(_M, "This should be non-null");
      // Attach our scaling/mass matrix to the PETSc object. PETSc will use this during the
      // preconditioner application
      ierr = PetscObjectCompose((PetscObject)lsc_pc_pmat, "LSC_Qscale", (PetscObject)_M);
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
  ns_problem->setupMatrices(schur_ksp);

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
  PetscErrorCode ierr = 0;
  KSP ksp;
  auto snes = currentNonlinearSystem().getSNES();
  ierr = SNESGetKSP(snes, &ksp);
  LIBMESH_CHKERR2(this->comm(), ierr);
  ierr = KSPSetPreSolve(ksp, &navierStokesKSPPreSolve, this);
  LIBMESH_CHKERR2(this->comm(), ierr);
}

#endif
