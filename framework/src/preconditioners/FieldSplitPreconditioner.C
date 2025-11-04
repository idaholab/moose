//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/petsc_macro.h"
#include "FieldSplitPreconditioner.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseEnum.h"
#include "MooseVariableFE.h"
#include "NonlinearSystem.h"
#include "PetscSupport.h"
#include "MoosePreconditioner.h"
#include "MooseStaticCondensationPreconditioner.h"
#include "Split.h"
#include "PetscDMMoose.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/coupling_matrix.h"

using namespace libMesh;

template <typename Base>
InputParameters
FieldSplitPreconditionerTempl<Base>::validParams()
{
  InputParameters params = Base::validParams();
  params.addClassDescription("Preconditioner designed to map onto PETSc's PCFieldSplit.");

  params.addRequiredParam<std::string>(
      "topsplit", "Entrance to splits, the top split will specify how splits will go.");
  // We should use full coupling Jacobian matrix by default
  params.addParam<bool>("full",
                        true,
                        "Set to true if you want the full set of couplings between variables "
                        "simply for convenience so you don't have to set every off_diag_row "
                        "and off_diag_column combination.");
  return params;
}

template <typename Base>
FieldSplitPreconditionerTempl<Base>::FieldSplitPreconditionerTempl(
    const InputParameters & parameters)
  : Base(parameters),
    _nl(this->_fe_problem.getNonlinearSystemBase(this->_nl_sys_num)),
    _decomposition_split(this->template getParam<std::string>("topsplit"))
{
  if (libMesh::default_solver_package() != libMesh::PETSC_SOLVERS)
    mooseError("The field split preconditioner can only be used with PETSc");

  // number of variables
  unsigned int n_vars = _nl.nVariables();
  // if we want to construct a full Jacobian?
  // it is recommended to have a full Jacobian for using
  // the fieldSplit preconditioner
  bool full = this->template getParam<bool>("full");

  // how variables couple
  std::unique_ptr<CouplingMatrix> cm = std::make_unique<CouplingMatrix>(n_vars);
  if (!full)
  {
    if (this->isParamValid("off_diag_row") && this->isParamValid("off_diag_column"))
    {

      const auto off_diag_rows =
          this->template getParam<std::vector<NonlinearVariableName>>("off_diag_row");
      const auto off_diag_columns =
          this->template getParam<std::vector<NonlinearVariableName>>("off_diag_column");

      // put 1s on diagonal
      for (unsigned int i = 0; i < n_vars; i++)
        (*cm)(i, i) = 1;

      // off-diagonal entries
      std::vector<std::vector<unsigned int>> off_diag(n_vars);
      if (off_diag_rows.size() * off_diag_columns.size() != 0 &&
          off_diag_rows.size() == off_diag_columns.size())
        for (const auto i : index_range(off_diag_rows))
        {
          unsigned int row = _nl.getVariable(0, off_diag_rows[i]).number();
          unsigned int column = _nl.getVariable(0, off_diag_columns[i]).number();
          (*cm)(row, column) = 1;
        }
    }
  }
  else
  {
    for (unsigned int i = 0; i < n_vars; i++)
      for (unsigned int j = 0; j < n_vars; j++)
        (*cm)(i, j) = 1; // full coupling
  }
  this->setCouplingMatrix(std::move(cm));

  // turn on a flag
  _nl.useFieldSplitPreconditioner(this);
}

template <typename Base>
void
FieldSplitPreconditionerTempl<Base>::createMooseDM(DM * dm)
{
  LibmeshPetscCallA(
      _nl.comm().get(),
      DMCreateMoose(_nl.comm().get(), _nl, dofMap(), system(), _decomposition_split, dm));
  LibmeshPetscCallA(_nl.comm().get(),
                    PetscObjectSetOptionsPrefix((PetscObject)*dm, prefix().c_str()));
  LibmeshPetscCall(DMSetFromOptions(*dm));
  LibmeshPetscCall(DMSetUp(*dm));
}

registerMooseObjectAliased("MooseApp", FieldSplitPreconditioner, "FSP");

InputParameters
FieldSplitPreconditioner::validParams()
{
  return FieldSplitPreconditionerTempl<MoosePreconditioner>::validParams();
}

FieldSplitPreconditioner::FieldSplitPreconditioner(const InputParameters & params)
  : FieldSplitPreconditionerTempl<MoosePreconditioner>(params)
{
  std::shared_ptr<Split> top_split = _nl.getSplit(_decomposition_split);
  top_split->setup(_nl, _nl.prefix());
}

void
FieldSplitPreconditioner::setupDM()
{
  PetscBool ismoose;
  DM dm = LIBMESH_PETSC_NULLPTR;

  // Initialize the part of the DM package that's packaged with Moose; in the PETSc source tree this
  // call would be in DMInitializePackage()
  LibmeshPetscCall(DMMooseRegisterAll());
  // Create and set up the DM that will consume the split options and deal with block matrices.
  auto * const petsc_solver =
      libMesh::cast_ptr<PetscNonlinearSolver<Number> *>(_nl.nonlinearSolver());
  SNES snes = petsc_solver->snes(prefix().c_str());
  // if there exists a DMMoose object, not to recreate a new one
  LibmeshPetscCall(SNESGetDM(snes, &dm));
  if (dm)
  {
    LibmeshPetscCall(PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose));
    if (ismoose)
      return;
  }
  createMooseDM(&dm);
  LibmeshPetscCall(SNESSetDM(snes, dm));
  LibmeshPetscCall(DMDestroy(&dm));
}

const DofMapBase &
FieldSplitPreconditioner::dofMap() const
{
  return _nl.dofMap();
}

const System &
FieldSplitPreconditioner::system() const
{
  return _nl.system();
}

std::string
FieldSplitPreconditioner::prefix() const
{
  return _nl.prefix();
}

KSP
FieldSplitPreconditioner::getKSP()
{
  KSP ksp;
  auto snes = _nl.getSNES();
  LibmeshPetscCall(SNESGetKSP(snes, &ksp));
  return ksp;
}

template class FieldSplitPreconditionerTempl<MooseStaticCondensationPreconditioner>;
