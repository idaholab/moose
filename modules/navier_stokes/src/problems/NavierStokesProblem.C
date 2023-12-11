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
  params.addRequiredParam<TagName>("mass_matrix",
                                   "The matrix tag name corresponding to the mass matrix.");
  params.addRequiredParam<TagName>(
      "L_matrix",
      "The matrix tag name corresponding to the diffusive part of the velocity equations.");
  params.addParam<std::vector<unsigned int>>(
      "schur_fs_index",
      {},
      "if not provided then the top field split is assumed to be the "
      "Schur split. This is a vector allow recursive nesting");
  params.addParam<bool>("use_pressure_mass_matrix",
                        false,
                        "Whether to just use the pressure mass matrix as the preconditioner for "
                        "the Schur complement");
  params.addParam<bool>(
      "commute_lsc",
      false,
      "Whether to use the commuted form of the LSC preconditioner, created by Olshanskii");
  params.addParam<bool>("use_mass_matrix_for_scaling",
                        true,
                        "Whether to use the mass matrix for scaling. This should always be true if "
                        "'commute_lsc' is true. If this is false (and 'commute_lsc' is false), "
                        "then the diagonal of A will be "
                        "used for scaling if scaling is requested.");
  return params;
}

NavierStokesProblem::NavierStokesProblem(const InputParameters & parameters)
  : FEProblem(parameters),
    _mass_matrix(getParam<TagName>("mass_matrix")),
    _L_matrix(getParam<TagName>("L_matrix")),
    _schur_fs_index(getParam<std::vector<unsigned int>>("schur_fs_index"))
{
  if (getParam<bool>("commute_lsc") && !getParam<bool>("use_mass_matrix_for_scaling"))
    paramError("use_mass_matrix_for_scaling",
               "This must be true if we are commuting the LSC commutator.");
}

NavierStokesProblem::~NavierStokesProblem()
{
  if (_Q_scale)
    // We're destructing so don't check for errors which can throw
    MatDestroy(&_Q_scale);
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

  ierr = KSPGetPC(node, &fs_pc);
  LIBMESH_CHKERR2(this->comm(), ierr);
  PetscObjectTypeCompare((PetscObject)fs_pc, PCFIELDSPLIT, &is_fs);
  LIBMESH_CHKERR2(this->comm(), ierr);
  if (!is_fs)
    mooseError("Not a field split. Please check the 'schur_fs_index' parameter");
  // Need to call this before getting the sub ksps
  ierr = PCSetUp(fs_pc);
  LIBMESH_CHKERR2(this->comm(), ierr);
  ierr = PCFieldSplitGetSubKSP(fs_pc, &num_splits, &subksp);
  LIBMESH_CHKERR2(this->comm(), ierr);
  next_ksp = subksp[sub_ksp_index];

  ierr = PCFieldSplitGetISByIndex(fs_pc, sub_ksp_index, &is);
  LIBMESH_CHKERR2(this->comm(), ierr);

  _index_sets.push_back(is);

  ierr = PetscFree(subksp);
  LIBMESH_CHKERR2(this->comm(), ierr);

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
  PetscErrorCode ierr = 0;

  ierr = KSPGetPC(schur_ksp, &schur_pc);
  LIBMESH_CHKERR2(this->comm(), ierr);
  PetscObjectTypeCompare((PetscObject)schur_pc, PCFIELDSPLIT, &is_fs);
  LIBMESH_CHKERR2(this->comm(), ierr);
  if (!is_fs)
    mooseError("Not a field split. Please check the 'schur_fs_index' parameter");

  // The mass matrix
  auto global_Q =
      static_cast<PetscMatrix<Number> &>(getNonlinearSystemBase(0).getMatrix(massMatrixTagID()))
          .mat();
  auto global_L =
      static_cast<PetscMatrix<Number> &>(getNonlinearSystemBase(0).getMatrix(LMatrixTagID())).mat();
  // The velocity block of the mass matrix
  auto & Q_scale = getQscale();
  auto & L = getL();

  auto process_intermediate_mats = [this, &ierr](auto & intermediate_mats, auto parent_mat)
  {
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

  auto our_parent_Q = process_intermediate_mats(intermediate_Qs, global_Q);
  auto our_parent_L = process_intermediate_mats(intermediate_Ls, global_L);

  // Need to call this before getting index sets or sub ksps, etc.
  ierr = PCSetUp(schur_pc);
  LIBMESH_CHKERR2(this->comm(), ierr);

  ierr = PCFieldSplitGetISByIndex(schur_pc, 0, &velocity_is);
  LIBMESH_CHKERR2(this->comm(), ierr);
  ierr = MatGetOwnershipRange(our_parent_Q, &rstart, &rend);
  LIBMESH_CHKERR2(this->comm(), ierr);

  const auto commute_lsc = getParam<bool>("commute_lsc");

  if (commute_lsc)
  {
    if (!L)
    {
      ierr = MatCreateSubMatrix(our_parent_L, velocity_is, velocity_is, MAT_INITIAL_MATRIX, &L);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
    else
    {
      ierr = MatCreateSubMatrix(our_parent_L, velocity_is, velocity_is, MAT_REUSE_MATRIX, &L);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
  }

  ierr = ISComplement(velocity_is, rstart, rend, &pressure_is);
  LIBMESH_CHKERR2(this->comm(), ierr);
  if (!Q_scale)
  {
    if (commute_lsc)
    {
      ierr =
          MatCreateSubMatrix(our_parent_Q, pressure_is, pressure_is, MAT_INITIAL_MATRIX, &Q_scale);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
    else
    {
      ierr =
          MatCreateSubMatrix(our_parent_Q, velocity_is, velocity_is, MAT_INITIAL_MATRIX, &Q_scale);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
  }
  else
  {
    if (commute_lsc)
    {
      ierr = MatCreateSubMatrix(our_parent_Q, pressure_is, pressure_is, MAT_REUSE_MATRIX, &Q_scale);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
    else
    {
      ierr = MatCreateSubMatrix(our_parent_Q, velocity_is, velocity_is, MAT_REUSE_MATRIX, &Q_scale);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
  }
  ierr = ISDestroy(&pressure_is);
  LIBMESH_CHKERR2(this->comm(), ierr);

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

  ierr = PCFieldSplitGetSubKSP(schur_pc, &num_splits, &subksp);
  LIBMESH_CHKERR2(this->comm(), ierr);
  if (num_splits != 2)
    mooseError("The number of splits should be two");
  schur_complement_ksp = subksp[1];

  if (getParam<bool>("use_pressure_mass_matrix"))
  {
    Mat S;
    ierr = PCFieldSplitSetSchurPre(schur_pc, PC_FIELDSPLIT_SCHUR_PRE_USER, Q_scale);
    LIBMESH_CHKERR2(this->comm(), ierr);
    ierr = KSPGetOperators(schur_complement_ksp, &S, NULL);
    LIBMESH_CHKERR2(this->comm(), ierr);
    ierr = KSPSetOperators(schur_complement_ksp, S, Q_scale);
    LIBMESH_CHKERR2(this->comm(), ierr);
  }
  else
  {
    ierr = KSPGetPC(schur_complement_ksp, &lsc_pc);
    LIBMESH_CHKERR2(this->comm(), ierr);
    ierr = PetscObjectTypeCompare(PetscObject(lsc_pc), PCLSC, &is_lsc);
    LIBMESH_CHKERR2(this->comm(), ierr);
    if (!is_lsc)
      mooseError("Not an LSC PC. Please check the 'schur_fs_index' parameter");
    ierr = PCGetOperators(lsc_pc, NULL, &lsc_pc_pmat);
    LIBMESH_CHKERR2(this->comm(), ierr);

    if (commute_lsc)
    {
      ierr = PetscObjectCompose((PetscObject)lsc_pc_pmat, "LSC_L", (PetscObject)L);
      LIBMESH_CHKERR2(this->comm(), ierr);
    }
    ierr = PetscObjectCompose((PetscObject)lsc_pc_pmat, "LSC_Qscale", (PetscObject)Q_scale);
    LIBMESH_CHKERR2(this->comm(), ierr);

    ierr = PetscFree(subksp);
    LIBMESH_CHKERR2(this->comm(), ierr);
  }
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
NavierStokesProblem::initPetscOutput()
{
  FEProblem::initPetscOutput();

  if (!getParam<bool>("commute_lsc") && !getParam<bool>("use_mass_matrix_for_scaling"))
    return;

  PetscErrorCode ierr = 0;
  KSP ksp;
  auto snes = getNonlinearSystemBase(0).getSNES();
  ierr = SNESGetKSP(snes, &ksp);
  LIBMESH_CHKERR2(this->comm(), ierr);
  ierr = KSPSetPreSolve(ksp, &navierStokesKSPPreSolve, this);
  LIBMESH_CHKERR2(this->comm(), ierr);
}
