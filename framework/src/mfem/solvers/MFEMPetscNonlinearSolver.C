//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPetscNonlinearSolver.h"
#include "MooseError.h"
#include "PetscSupport.h"
#include "MFEMProblem.h"

#ifdef MFEM_USE_PETSC

registerMooseObject("MooseApp", MFEMPetscNonlinearSolver);

InputParameters
MFEMPetscNonlinearSolver::validParams()
{
  InputParameters params = Moose::MFEM::NonlinearSolverBase::validParams();
  params.addClassDescription("MFEM PETSc-backed nonlinear solver using SNES.");
  params.addParam<MultiMooseEnum>(
      "petsc_options", Moose::PetscSupport::getCommonPetscFlags(), "Singleton PETSc options");
  params.addParam<MultiMooseEnum>("petsc_options_iname",
                                  Moose::PetscSupport::getCommonPetscKeys(),
                                  "Names of PETSc name/value pairs");
  params.addParam<std::vector<std::string>>(
      "petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\")");
  params.addParam<std::string>(
      "petsc_options_prefix", "", "PETSc options prefix used for this nonlinear solver.");
  return params;
}

MFEMPetscNonlinearSolver::MFEMPetscNonlinearSolver(const InputParameters & parameters)
  : Moose::MFEM::NonlinearSolverBase(parameters)
{
  constructSolver();
}

void
MFEMPetscNonlinearSolver::constructSolver()
{
  const auto & prefix = getParam<std::string>("petsc_options_prefix");
  const auto normalized_prefix = !prefix.empty() && prefix.back() != '_' ? prefix + "_" : prefix;

  Moose::PetscSupport::PetscOptions petsc_options;
  Moose::PetscSupport::addPetscFlagsToPetscOptions(
      getParam<MultiMooseEnum>("petsc_options"), normalized_prefix, *this, petsc_options);
  Moose::PetscSupport::addPetscPairsToPetscOptions(
      getParam<MooseEnumItem, std::string>("petsc_options_iname", "petsc_options_value"),
      getMFEMProblem().mesh().dimension(),
      normalized_prefix,
      *this,
      petsc_options);

  for (const auto & flag : petsc_options.flags)
    Moose::PetscSupport::setSinglePetscOption(flag.rawName().c_str());
  for (const auto & option : petsc_options.pairs)
    Moose::PetscSupport::setSinglePetscOption(option.first, option.second);

  auto solver =
      std::make_unique<mfem::PetscNonlinearSolver>(getMFEMProblem().getComm(), normalized_prefix);
  solver->iterative_mode = getParam<bool>("use_initial_guess");
  solver->SetRelTol(getParam<mfem::real_t>("rel_tol"));
  solver->SetAbsTol(getParam<mfem::real_t>("abs_tol"));
  solver->SetMaxIter(getParam<unsigned int>("max_its"));
  solver->SetPrintLevel(getParam<unsigned int>("print_level"));
  solver->SetJacobianType(mfem::Operator::PETSC_MATAIJ);
  _solver = std::move(solver);
}

void
MFEMPetscNonlinearSolver::SetOperator(const mfem::Operator & op)
{
  static_cast<mfem::PetscNonlinearSolver &>(getSolver()).SetOperator(op);
}

void
MFEMPetscNonlinearSolver::SetLinearSolver(mfem::Solver &)
{
  // mfem::PetscNonlinearSolver owns its internal SNES/KSP stack and does not expose
  // an API for injecting an external mfem::Solver-backed linear solver.
}

void
MFEMPetscNonlinearSolver::Mult(const mfem::Vector & rhs, mfem::Vector & x)
{
  getSolver().Mult(rhs, x);
}
#endif

#endif
