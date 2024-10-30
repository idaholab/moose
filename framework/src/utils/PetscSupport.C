//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PetscSupport.h"

// MOOSE includes
#include "MooseApp.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "NonlinearSystem.h"
#include "LinearSystem.h"
#include "DisplacedProblem.h"
#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"
#include "MooseTypes.h"
#include "MooseUtils.h"
#include "CommandLine.h"
#include "Console.h"
#include "MultiMooseEnum.h"
#include "Conversion.h"
#include "Executioner.h"
#include "MooseMesh.h"
#include "ComputeLineSearchObjectWrapper.h"
#include "Convergence.h"

#include "libmesh/equation_systems.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/petsc_linear_solver.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/petsc_preconditioner.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_solver_exception.h"
#include "libmesh/simple_range.h"

// PETSc includes
#include <petsc.h>
#include <petscsnes.h>
#include <petscksp.h>

// For graph coloring
#include <petscmat.h>
#include <petscis.h>
#include <petscdm.h>

// PetscDMMoose include
#include "PetscDMMoose.h"

// Standard includes
#include <ostream>
#include <fstream>
#include <string>

using namespace libMesh;

void
MooseVecView(NumericVector<Number> & vector)
{
  PetscVector<Number> & petsc_vec = static_cast<PetscVector<Number> &>(vector);
  LibmeshPetscCallA(vector.comm().get(), VecView(petsc_vec.vec(), 0));
}

void
MooseMatView(SparseMatrix<Number> & mat)
{
  PetscMatrix<Number> & petsc_mat = static_cast<PetscMatrix<Number> &>(mat);
  LibmeshPetscCallA(mat.comm().get(), MatView(petsc_mat.mat(), 0));
}

void
MooseVecView(const NumericVector<Number> & vector)
{
  PetscVector<Number> & petsc_vec =
      static_cast<PetscVector<Number> &>(const_cast<NumericVector<Number> &>(vector));
  LibmeshPetscCallA(vector.comm().get(), VecView(petsc_vec.vec(), 0));
}

void
MooseMatView(const SparseMatrix<Number> & mat)
{
  PetscMatrix<Number> & petsc_mat =
      static_cast<PetscMatrix<Number> &>(const_cast<SparseMatrix<Number> &>(mat));
  LibmeshPetscCallA(mat.comm().get(), MatView(petsc_mat.mat(), 0));
}

namespace Moose
{
namespace PetscSupport
{

std::string
stringify(const LineSearchType & t)
{
  switch (t)
  {
    case LS_BASIC:
      return "basic";
    case LS_DEFAULT:
      return "default";
    case LS_NONE:
      return "none";
    case LS_SHELL:
      return "shell";
    case LS_L2:
      return "l2";
    case LS_BT:
      return "bt";
    case LS_CP:
      return "cp";
    case LS_CONTACT:
      return "contact";
    case LS_PROJECT:
      return "project";
    case LS_INVALID:
      mooseError("Invalid LineSearchType");
  }
  return "";
}

std::string
stringify(const MffdType & t)
{
  switch (t)
  {
    case MFFD_WP:
      return "wp";
    case MFFD_DS:
      return "ds";
    case MFFD_INVALID:
      mooseError("Invalid MffdType");
  }
  return "";
}

void
setSolverOptions(const SolverParams & solver_params)
{
  // set PETSc options implied by a solve type
  switch (solver_params._type)
  {
    case Moose::ST_PJFNK:
      setSinglePetscOption("-snes_mf_operator");
      setSinglePetscOption("-mat_mffd_type", stringify(solver_params._mffd_type));
      break;

    case Moose::ST_JFNK:
      setSinglePetscOption("-snes_mf");
      setSinglePetscOption("-mat_mffd_type", stringify(solver_params._mffd_type));
      break;

    case Moose::ST_NEWTON:
      break;

    case Moose::ST_FD:
      setSinglePetscOption("-snes_fd");
      break;

    case Moose::ST_LINEAR:
      setSinglePetscOption("-snes_type", "ksponly");
      setSinglePetscOption("-snes_monitor_cancel");
      break;
  }

  Moose::LineSearchType ls_type = solver_params._line_search;
  if (ls_type == Moose::LS_NONE)
    ls_type = Moose::LS_BASIC;

  if (ls_type != Moose::LS_DEFAULT && ls_type != Moose::LS_CONTACT && ls_type != Moose::LS_PROJECT)
    setSinglePetscOption("-snes_linesearch_type", stringify(ls_type));
}

void
petscSetupDM(NonlinearSystemBase & nl, const std::string & dm_name)
{
  PetscBool ismoose;
  DM dm = LIBMESH_PETSC_NULLPTR;

  // Initialize the part of the DM package that's packaged with Moose; in the PETSc source tree this
  // call would be in DMInitializePackage()
  LibmeshPetscCallA(nl.comm().get(), DMMooseRegisterAll());
  // Create and set up the DM that will consume the split options and deal with block matrices.
  PetscNonlinearSolver<Number> * petsc_solver =
      dynamic_cast<PetscNonlinearSolver<Number> *>(nl.nonlinearSolver());
  SNES snes = petsc_solver->snes();
  // if there exists a DMMoose object, not to recreate a new one
  LibmeshPetscCallA(nl.comm().get(), SNESGetDM(snes, &dm));
  if (dm)
  {
    LibmeshPetscCallA(nl.comm().get(), PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose));
    if (ismoose)
      return;
  }
  LibmeshPetscCallA(nl.comm().get(), DMCreateMoose(nl.comm().get(), nl, dm_name, &dm));
  LibmeshPetscCallA(nl.comm().get(), DMSetFromOptions(dm));
  LibmeshPetscCallA(nl.comm().get(), DMSetUp(dm));
  LibmeshPetscCallA(nl.comm().get(), SNESSetDM(snes, dm));
  LibmeshPetscCallA(nl.comm().get(), DMDestroy(&dm));
  // We temporarily comment out this updating function because
  // we lack an approach to check if the problem
  // structure has been changed from the last iteration.
  // The indices will be rebuilt for every timestep.
  // TODO: figure out a way to check the structure changes of the
  // matrix
  // ierr = SNESSetUpdate(snes,SNESUpdateDMMoose);
  // CHKERRABORT(nl.comm().get(),ierr);
}

void
addPetscOptionsFromCommandline()
{
  // commandline options always win
  // the options from a user commandline will overwrite the existing ones if any conflicts
  { // Get any options specified on the command-line
    int argc;
    char ** args;

    LibmeshPetscCallA(PETSC_COMM_WORLD, PetscGetArgs(&argc, &args));
#if PETSC_VERSION_LESS_THAN(3, 7, 0)
    LibmeshPetscCallA(PETSC_COMM_WORLD, PetscOptionsInsert(&argc, &args, NULL));
#else
    LibmeshPetscCallA(PETSC_COMM_WORLD,
                      PetscOptionsInsert(LIBMESH_PETSC_NULLPTR, &argc, &args, NULL));
#endif
  }
}

void
petscSetOptions(const PetscOptions & po,
                const SolverParams & solver_params,
                FEProblemBase * const problem /*=nullptr*/)
{
#if PETSC_VERSION_LESS_THAN(3, 7, 0)
  LibmeshPetscCallA(PETSC_COMM_WORLD, PetscOptionsClear());
#else
  LibmeshPetscCallA(PETSC_COMM_WORLD, PetscOptionsClear(LIBMESH_PETSC_NULLPTR));
#endif

  setSolverOptions(solver_params);

  // Add any additional options specified in the input file
  for (const auto & flag : po.flags)
    setSinglePetscOption(flag.rawName().c_str());

  // Add option pairs
  for (auto & option : po.pairs)
    setSinglePetscOption(option.first, option.second, problem);

  addPetscOptionsFromCommandline();
}

PetscErrorCode
petscSetupOutput(CommandLine * cmd_line)
{
  PetscFunctionBegin;
  char code[10] = {45, 45, 109, 111, 111, 115, 101};
  const std::vector<std::string> argv = cmd_line->getArguments();
  for (const auto & arg : argv)
  {
    if (arg.compare(code) == 0)
    {
      Console::petscSetupOutput();
      break;
    }
  }
  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
petscNonlinearConverged(SNES /*snes*/,
                        PetscInt it,
                        PetscReal /*xnorm*/,
                        PetscReal /*snorm*/,
                        PetscReal /*fnorm*/,
                        SNESConvergedReason * reason,
                        void * ctx)
{
  PetscFunctionBegin;
  FEProblemBase & problem = *static_cast<FEProblemBase *>(ctx);

  // execute objects that may be used in convergence check
  problem.execute(EXEC_NONLINEAR_CONVERGENCE);

  // perform the convergence check
  Convergence::MooseConvergenceStatus status;
  if (problem.getFailNextNonlinearConvergenceCheck())
  {
    status = Convergence::MooseConvergenceStatus::DIVERGED;
    problem.resetFailNextNonlinearConvergenceCheck();
  }
  else
  {
    auto & convergence = problem.getConvergence(
        problem.getNonlinearConvergenceNames()[problem.currentNonlinearSystem().number()]);
    status = convergence.checkConvergence(it);
  }

  // convert convergence status to PETSc converged reason
  switch (status)
  {
    case Convergence::MooseConvergenceStatus::ITERATING:
      *reason = SNES_CONVERGED_ITERATING;
      break;

    case Convergence::MooseConvergenceStatus::CONVERGED:
      *reason = SNES_CONVERGED_FNORM_ABS;
      break;

    case Convergence::MooseConvergenceStatus::DIVERGED:
      *reason = SNES_DIVERGED_DTOL;
      break;
  }

  PetscFunctionReturn(PETSC_SUCCESS);
}

PCSide
getPetscPCSide(Moose::PCSideType pcs)
{
  switch (pcs)
  {
    case Moose::PCS_LEFT:
      return PC_LEFT;
    case Moose::PCS_RIGHT:
      return PC_RIGHT;
    case Moose::PCS_SYMMETRIC:
      return PC_SYMMETRIC;
    default:
      mooseError("Unknown PC side requested.");
      break;
  }
}

KSPNormType
getPetscKSPNormType(Moose::MooseKSPNormType kspnorm)
{
  switch (kspnorm)
  {
    case Moose::KSPN_NONE:
      return KSP_NORM_NONE;
    case Moose::KSPN_PRECONDITIONED:
      return KSP_NORM_PRECONDITIONED;
    case Moose::KSPN_UNPRECONDITIONED:
      return KSP_NORM_UNPRECONDITIONED;
    case Moose::KSPN_NATURAL:
      return KSP_NORM_NATURAL;
    case Moose::KSPN_DEFAULT:
      return KSP_NORM_DEFAULT;
    default:
      mooseError("Unknown KSP norm type requested.");
      break;
  }
}

void
petscSetDefaultKSPNormType(FEProblemBase & problem, KSP ksp)
{
  for (const auto i : make_range(problem.numSolverSystems()))
  {
    SolverSystem & sys = problem.getSolverSystem(i);
    LibmeshPetscCallA(problem.comm().get(),
                      KSPSetNormType(ksp, getPetscKSPNormType(sys.getMooseKSPNormType())));
  }
}

void
petscSetDefaultPCSide(FEProblemBase & problem, KSP ksp)
{
  for (const auto i : make_range(problem.numSolverSystems()))
  {
    SolverSystem & sys = problem.getSolverSystem(i);

    // PETSc 3.2.x+
    if (sys.getPCSide() != Moose::PCS_DEFAULT)
      LibmeshPetscCallA(problem.comm().get(), KSPSetPCSide(ksp, getPetscPCSide(sys.getPCSide())));
  }
}

void
petscSetKSPDefaults(FEProblemBase & problem, KSP ksp)
{
  auto & es = problem.es();

  PetscReal rtol = es.parameters.get<Real>("linear solver tolerance");
  PetscReal atol = es.parameters.get<Real>("linear solver absolute tolerance");

  // MOOSE defaults this to -1 for some dumb reason
  if (atol < 0)
    atol = 1e-50;

  PetscReal maxits = es.parameters.get<unsigned int>("linear solver maximum iterations");

  // 1e100 is because we don't use divtol currently
  LibmeshPetscCallA(problem.comm().get(), KSPSetTolerances(ksp, rtol, atol, 1e100, maxits));

  petscSetDefaultPCSide(problem, ksp);

  petscSetDefaultKSPNormType(problem, ksp);
}

void
petscSetDefaults(FEProblemBase & problem)
{
  for (auto nl_index : make_range(problem.numNonlinearSystems()))
  {
    // dig out PETSc solver
    NonlinearSystemBase & nl = problem.getNonlinearSystemBase(nl_index);
    auto * const petsc_solver = cast_ptr<PetscNonlinearSolver<Number> *>(nl.nonlinearSolver());
    auto * const sys_matrix = petsc_solver->system().request_matrix("System Matrix");
    // Prefix the name of the system matrix with the name of the system
    if (sys_matrix && problem.solverParams()._type != Moose::ST_JFNK)
    {
      auto * const petsc_sys_matrix = cast_ptr<PetscMatrix<Number> *>(sys_matrix);
      LibmeshPetscCall2(
          nl.comm(),
          MatSetOptionsPrefix(petsc_sys_matrix->mat(),
                              (problem.getNonlinearSystemNames()[nl_index] + "_").c_str()));
      LibmeshPetscCall2(nl.comm(), MatSetFromOptions(petsc_sys_matrix->mat()));
    }
    SNES snes = petsc_solver->snes();
    KSP ksp;
    LibmeshPetscCallA(nl.comm().get(), SNESGetKSP(snes, &ksp));

    LibmeshPetscCallA(nl.comm().get(), SNESSetMaxLinearSolveFailures(snes, 1000000));

    LibmeshPetscCallA(nl.comm().get(), SNESSetCheckJacobianDomainError(snes, PETSC_TRUE));

    // In 3.0.0, the context pointer must actually be used, and the
    // final argument to KSPSetConvergenceTest() is a pointer to a
    // routine for destroying said private data context.  In this case,
    // we use the default context provided by PETSc in addition to
    // a few other tests.
    {
      LibmeshPetscCallA(
          nl.comm().get(),
          SNESSetConvergenceTest(snes, petscNonlinearConverged, &problem, LIBMESH_PETSC_NULLPTR));
    }

    petscSetKSPDefaults(problem, ksp);
  }

  for (auto sys_index : make_range(problem.numLinearSystems()))
  {
    // dig out PETSc solver
    LinearSystem & lin_sys = problem.getLinearSystem(sys_index);
    PetscLinearSolver<Number> * petsc_solver = dynamic_cast<PetscLinearSolver<Number> *>(
        lin_sys.linearImplicitSystem().get_linear_solver());
    KSP ksp = petsc_solver->ksp();
    petscSetKSPDefaults(problem, ksp);
  }
}

void
storePetscOptions(FEProblemBase & fe_problem, const InputParameters & params)
{
  // Note: Options set in the Preconditioner block will override those set in the Executioner block
  if (params.isParamValid("solve_type") && !params.isParamValid("_use_eigen_value"))
  {
    // Extract the solve type
    const std::string & solve_type = params.get<MooseEnum>("solve_type");
    fe_problem.solverParams()._type = Moose::stringToEnum<Moose::SolveType>(solve_type);
  }

  if (params.isParamValid("line_search"))
  {
    MooseEnum line_search = params.get<MooseEnum>("line_search");
    if (fe_problem.solverParams()._line_search == Moose::LS_INVALID || line_search != "default")
    {
      Moose::LineSearchType enum_line_search =
          Moose::stringToEnum<Moose::LineSearchType>(line_search);
      fe_problem.solverParams()._line_search = enum_line_search;
      if (enum_line_search == LS_CONTACT || enum_line_search == LS_PROJECT)
        for (auto nl_index : make_range(fe_problem.numNonlinearSystems()))
        {
          NonlinearImplicitSystem * nl_system = dynamic_cast<NonlinearImplicitSystem *>(
              &fe_problem.getNonlinearSystemBase(nl_index).system());
          if (!nl_system)
            mooseError("You've requested a line search but you must be solving an EigenProblem. "
                       "These two things are not consistent.");
          PetscNonlinearSolver<Real> * petsc_nonlinear_solver =
              dynamic_cast<PetscNonlinearSolver<Real> *>(nl_system->nonlinear_solver.get());
          if (!petsc_nonlinear_solver)
            mooseError("Currently the MOOSE line searches all use Petsc, so you "
                       "must use Petsc as your non-linear solver.");
          petsc_nonlinear_solver->linesearch_object =
              std::make_unique<ComputeLineSearchObjectWrapper>(fe_problem);
        }
    }
  }

  if (params.isParamValid("mffd_type"))
  {
    MooseEnum mffd_type = params.get<MooseEnum>("mffd_type");
    fe_problem.solverParams()._mffd_type = Moose::stringToEnum<Moose::MffdType>(mffd_type);
  }

  // The parameters contained in the Action
  const auto & petsc_options = params.get<MultiMooseEnum>("petsc_options");
  const auto & petsc_pair_options =
      params.get<MooseEnumItem, std::string>("petsc_options_iname", "petsc_options_value");

  // A reference to the PetscOptions object that contains the settings that will be used in the
  // solve
  Moose::PetscSupport::PetscOptions & po = fe_problem.getPetscOptions();

  // First process the single petsc options/flags
  processPetscFlags(petsc_options, po);

  // Then process the option-value pairs
  processPetscPairs(petsc_pair_options, fe_problem.mesh().dimension(), po);
}

void
processPetscFlags(const MultiMooseEnum & petsc_flags, PetscOptions & po)
{
  // Update the PETSc single flags
  for (const auto & option : petsc_flags)
  {
    /**
     * "-log_summary" cannot be used in the input file. This option needs to be set when PETSc is
     * initialized
     * which happens before the parser is even created.  We'll throw an error if somebody attempts
     * to add this option later.
     */
    if (option == "-log_summary" || option == "-log_view")
      mooseError("The PETSc option \"-log_summary\" or \"-log_view\" can only be used on the "
                 "command line.  Please "
                 "remove it from the input file");

    // Warn about superseded PETSc options (Note: -snes is not a REAL option, but people used it in
    // their input files)
    else
    {
      std::string help_string;
      if (option == "-snes" || option == "-snes_mf" || option == "-snes_mf_operator")
        help_string = "Please set the solver type through \"solve_type\".";
      else if (option == "-ksp_monitor")
        help_string = "Please use \"Outputs/print_linear_residuals=true\"";

      if (help_string != "")
        mooseWarning("The PETSc option ",
                     std::string(option),
                     " should not be used directly in a MOOSE input file. ",
                     help_string);
    }

    // Update the stored items, but do not create duplicates
    if (!po.flags.isValueSet(option))
      po.flags.setAdditionalValue(option);
  }
}

void
processPetscPairs(const std::vector<std::pair<MooseEnumItem, std::string>> & petsc_pair_options,
                  const unsigned int mesh_dimension,
                  PetscOptions & po)
{
  // the boolean in these pairs denote whether the user has specified any of the reason flags in the
  // input file
  std::array<std::pair<bool, std::string>, 2> reason_flags = {
      {std::make_pair(false, "-snes_converged_reason"),
       std::make_pair(false, "-ksp_converged_reason")}};

  for (auto & reason_flag : reason_flags)
    if (po.flags.isValueSet(reason_flag.second))
      // We register the reason option as already existing
      reason_flag.first = true;

  // Setup the name value pairs
  bool boomeramg_found = false;
  bool strong_threshold_found = false;
#if !PETSC_VERSION_LESS_THAN(3, 7, 0)
  bool superlu_dist_found = false;
  bool fact_pattern_found = false;
  bool tiny_pivot_found = false;
#endif
  std::string pc_description = "";
#if !PETSC_VERSION_LESS_THAN(3, 12, 0)
  // If users use HMG, we would like to set
  bool hmg_found = false;
  bool matptap_found = false;
  bool hmg_strong_threshold_found = false;
#endif
  std::vector<std::pair<std::string, std::string>> new_options;

  for (const auto & option : petsc_pair_options)
  {
    new_options.clear();

    // Do not add duplicate settings
    if (MooseUtils::findPair(po.pairs, option.first, MooseUtils::Any) == po.pairs.end())
    {
#if !PETSC_VERSION_LESS_THAN(3, 9, 0)
      if (option.first == "-pc_factor_mat_solver_package")
        new_options.emplace_back("-pc_factor_mat_solver_type", option.second);
#else
      if (option.first == "-pc_factor_mat_solver_type")
        new_options.push_back("-pc_factor_mat_solver_package", option.second);
#endif

      // Look for a pc description
      if (option.first == "-pc_type" || option.first == "-pc_sub_type" ||
          option.first == "-pc_hypre_type")
        pc_description += option.second + ' ';

#if !PETSC_VERSION_LESS_THAN(3, 12, 0)
      if (option.first == "-pc_type" && option.second == "hmg")
        hmg_found = true;

        // MPIAIJ for PETSc 3.12.0: -matptap_via
        // MAIJ for PETSc 3.12.0: -matmaijptap_via
        // MPIAIJ for PETSc 3.13 to 3.16: -matptap_via, -matproduct_ptap_via
        // MAIJ for PETSc 3.13 to 3.16: -matproduct_ptap_via
        // MPIAIJ for PETSc 3.17 and higher: -matptap_via, -mat_product_algorithm
        // MAIJ for PETSc 3.17 and higher: -mat_product_algorithm
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
      if (hmg_found && (option.first == "-matptap_via" || option.first == "-matmaijptap_via" ||
                        option.first == "-matproduct_ptap_via"))
        new_options.emplace_back("-mat_product_algorithm", option.second);
#elif !PETSC_VERSION_LESS_THAN(3, 13, 0)
      if (hmg_found && (option.first == "-matptap_via" || option.first == "-matmaijptap_via"))
        new_options.emplace_back("-matproduct_ptap_via", option.second);
#else
      if (hmg_found && (option.first == "-matproduct_ptap_via"))
      {
        new_options.emplace_back("-matptap_via", option.second);
        new_options.emplace_back("-matmaijptap_via", option.second);
      }
#endif

      if (option.first == "-matptap_via" || option.first == "-matmaijptap_via" ||
          option.first == "-matproduct_ptap_via" || option.first == "-mat_product_algorithm")
        matptap_found = true;

      // For 3D problems, we need to set this 0.7
      if (option.first == "-hmg_inner_pc_hypre_boomeramg_strong_threshold")
        hmg_strong_threshold_found = true;
#endif
      // This special case is common enough that we'd like to handle it for the user.
      if (option.first == "-pc_hypre_type" && option.second == "boomeramg")
        boomeramg_found = true;
      if (option.first == "-pc_hypre_boomeramg_strong_threshold")
        strong_threshold_found = true;
#if !PETSC_VERSION_LESS_THAN(3, 7, 0)
      if ((option.first == "-pc_factor_mat_solver_package" ||
           option.first == "-pc_factor_mat_solver_type") &&
          option.second == "superlu_dist")
        superlu_dist_found = true;
      if (option.first == "-mat_superlu_dist_fact")
        fact_pattern_found = true;
      if (option.first == "-mat_superlu_dist_replacetinypivot")
        tiny_pivot_found = true;
#endif

      if (!new_options.empty())
        std::copy(new_options.begin(), new_options.end(), std::back_inserter(po.pairs));
      else
        po.pairs.push_back(option);
    }
    else
    {
      for (unsigned int j = 0; j < po.pairs.size(); j++)
        if (option.first == po.pairs[j].first)
          po.pairs[j].second = option.second;
    }
  }

#if !PETSC_VERSION_LESS_THAN(3, 14, 0)
  for (const auto & reason_flag : reason_flags)
    // Was the option already found in PetscOptions::flags? Or does it exist in PetscOptions::pairs
    // as an iname already? If not, then we add our flag
    if (!reason_flag.first && (std::find_if(po.pairs.begin(),
                                            po.pairs.end(),
                                            [&reason_flag](auto & pair) {
                                              return pair.first == reason_flag.second;
                                            }) == po.pairs.end()))
      po.pairs.emplace_back(reason_flag.second, "::failed");
#endif

  // When running a 3D mesh with boomeramg, it is almost always best to supply a strong threshold
  // value. We will provide that for the user here if they haven't supplied it themselves.
  if (boomeramg_found && !strong_threshold_found && mesh_dimension == 3)
  {
    po.pairs.emplace_back("-pc_hypre_boomeramg_strong_threshold", "0.7");
    pc_description += "strong_threshold: 0.7 (auto)";
  }

#if !PETSC_VERSION_LESS_THAN(3, 12, 0)
  if (hmg_found && !hmg_strong_threshold_found && mesh_dimension == 3)
  {
    po.pairs.emplace_back("-hmg_inner_pc_hypre_boomeramg_strong_threshold", "0.7");
    pc_description += "strong_threshold: 0.7 (auto)";
  }

  // Default PETSc PtAP takes too much memory, and it is not quite useful
  // Let us switch to use new algorithm
  if (hmg_found && !matptap_found)
  {
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
    po.pairs.emplace_back("-mat_product_algorithm", "allatonce");
#elif !PETSC_VERSION_LESS_THAN(3, 13, 0)
    po.pairs.emplace_back("-matproduct_ptap_via", "allatonce");
#else
    po.pairs.emplace_back("-matptap_via", "allatonce");
    po.pairs.emplace_back("-matmaijptap_via", "allatonce");
#endif
  }
#endif

#if !PETSC_VERSION_LESS_THAN(3, 7, 0)
  // In PETSc-3.7.{0--4}, there is a bug when using superlu_dist, and we have to use
  // SamePattern_SameRowPerm, otherwise we use whatever we have in PETSc
  if (superlu_dist_found && !fact_pattern_found)
  {
    po.pairs.emplace_back("-mat_superlu_dist_fact",
#if PETSC_VERSION_LESS_THAN(3, 7, 5)
                          "SamePattern_SameRowPerm");
    pc_description += "mat_superlu_dist_fact: SamePattern_SameRowPerm ";
#else
                          "SamePattern");
    pc_description += "mat_superlu_dist_fact: SamePattern ";
#endif
  }

  // restore this superlu  option
  if (superlu_dist_found && !tiny_pivot_found)
  {
    po.pairs.emplace_back("-mat_superlu_dist_replacetinypivot", "1");
    pc_description += " mat_superlu_dist_replacetinypivot: true ";
  }
#endif
  // Set Preconditioner description
  po.pc_description += pc_description;
}

std::set<std::string>
getPetscValidLineSearches()
{
  return {"default", "shell", "none", "basic", "l2", "bt", "cp"};
}

InputParameters
getPetscValidParams()
{
  InputParameters params = emptyInputParameters();

  MooseEnum solve_type("PJFNK JFNK NEWTON FD LINEAR");
  params.addParam<MooseEnum>("solve_type",
                             solve_type,
                             "PJFNK: Preconditioned Jacobian-Free Newton Krylov "
                             "JFNK: Jacobian-Free Newton Krylov "
                             "NEWTON: Full Newton Solve "
                             "FD: Use finite differences to compute Jacobian "
                             "LINEAR: Solving a linear problem");

  MooseEnum mffd_type("wp ds", "wp");
  params.addParam<MooseEnum>("mffd_type",
                             mffd_type,
                             "Specifies the finite differencing type for "
                             "Jacobian-free solve types. Note that the "
                             "default is wp (for Walker and Pernice).");

  params.addParam<MultiMooseEnum>(
      "petsc_options", getCommonPetscFlags(), "Singleton PETSc options");
  params.addParam<MultiMooseEnum>(
      "petsc_options_iname", getCommonPetscKeys(), "Names of PETSc name/value pairs");
  params.addParam<std::vector<std::string>>(
      "petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\"");
  params.addParamNamesToGroup("solve_type petsc_options petsc_options_iname petsc_options_value "
                              "mffd_type",
                              "PETSc");

  return params;
}

MultiMooseEnum
getCommonPetscFlags()
{
  return MultiMooseEnum(
      "-dm_moose_print_embedding -dm_view -ksp_converged_reason -ksp_gmres_modifiedgramschmidt "
      "-ksp_monitor -ksp_monitor_snes_lg-snes_ksp_ew -ksp_snes_ew -snes_converged_reason "
      "-snes_ksp -snes_ksp_ew -snes_linesearch_monitor -snes_mf -snes_mf_operator -snes_monitor "
      "-snes_test_display -snes_view",
      "",
      true);
}

MultiMooseEnum
getCommonPetscKeys()
{
  return MultiMooseEnum("-ksp_atol -ksp_gmres_restart -ksp_max_it -ksp_pc_side -ksp_rtol "
                        "-ksp_type -mat_fd_coloring_err -mat_fd_type -mat_mffd_type "
                        "-pc_asm_overlap -pc_factor_levels "
                        "-pc_factor_mat_ordering_type -pc_hypre_boomeramg_grid_sweeps_all "
                        "-pc_hypre_boomeramg_max_iter "
                        "-pc_hypre_boomeramg_strong_threshold -pc_hypre_type -pc_type -snes_atol "
                        "-snes_linesearch_type "
                        "-snes_ls -snes_max_it -snes_rtol -snes_divergence_tolerance -snes_type "
                        "-sub_ksp_type -sub_pc_type",
                        "",
                        true);
}

bool
isSNESVI(FEProblemBase & fe_problem)
{
  PetscOptions & petsc = fe_problem.getPetscOptions();

  int argc;
  char ** args;
  LibmeshPetscCallA(fe_problem.comm().get(), PetscGetArgs(&argc, &args));

  std::vector<std::string> cml_arg;
  for (int i = 0; i < argc; i++)
    cml_arg.push_back(args[i]);

  if (MooseUtils::findPair(petsc.pairs, MooseUtils::Any, "vinewtonssls") == petsc.pairs.end() &&
      MooseUtils::findPair(petsc.pairs, MooseUtils::Any, "vinewtonrsls") == petsc.pairs.end() &&
      std::find(cml_arg.begin(), cml_arg.end(), "vinewtonssls") == cml_arg.end() &&
      std::find(cml_arg.begin(), cml_arg.end(), "vinewtonrsls") == cml_arg.end())
    return false;

  return true;
}

void
setSinglePetscOption(const std::string & name,
                     const std::string & value,
                     FEProblemBase * const problem /*=nullptr*/)
{
  static const TIMPI::Communicator comm_world(PETSC_COMM_WORLD);
  const TIMPI::Communicator & comm = problem ? problem->comm() : comm_world;
  LibmeshPetscCall2(comm,
                    PetscOptionsSetValue(LIBMESH_PETSC_NULLPTR,
                                         name.c_str(),
                                         value == "" ? LIBMESH_PETSC_NULLPTR : value.c_str()));
  const auto lower_case_name = MooseUtils::toLower(name);
  auto check_problem = [problem, &lower_case_name]()
  {
    if (!problem)
      mooseError(
          "Setting the option '",
          lower_case_name,
          "' requires passing a 'problem' parameter. Contact a developer of your application "
          "to have them update their code. If in doubt, reach out to the MOOSE team on Github "
          "discussions");
  };

  // Select vector type from user-passed PETSc options
  if (lower_case_name.find("-vec_type") != std::string::npos)
  {
    check_problem();
    for (auto & solver_system : problem->_solver_systems)
    {
      auto & lm_sys = solver_system->system();
      for (auto & [vec_name, vec] : as_range(lm_sys.vectors_begin(), lm_sys.vectors_end()))
      {
        libmesh_ignore(vec_name);
        auto * const petsc_vec = cast_ptr<PetscVector<Number> *>(vec.get());
        LibmeshPetscCall2(comm, VecSetFromOptions(petsc_vec->vec()));
      }
      // The solution vectors aren't included in the system vectors storage
      auto * petsc_vec = cast_ptr<PetscVector<Number> *>(lm_sys.solution.get());
      LibmeshPetscCall2(comm, VecSetFromOptions(petsc_vec->vec()));
      petsc_vec = cast_ptr<PetscVector<Number> *>(lm_sys.current_local_solution.get());
      LibmeshPetscCall2(comm, VecSetFromOptions(petsc_vec->vec()));
    }
  }
  // Select matrix type from user-passed PETSc options
  else if (lower_case_name.find("mat_type") != std::string::npos)
  {
    check_problem();
    if (problem->solverParams()._type == Moose::ST_JFNK)
      mooseError("Setting option '", lower_case_name, "' is incompatible with a JFNK 'solve_type'");

    bool found_matching_prefix = false;
    for (const auto i : index_range(problem->_solver_systems))
    {
      const auto & prefix = problem->_solver_sys_names[i];
      if (lower_case_name.find("-" + MooseUtils::toLower(prefix) + "_mat_type") ==
          std::string::npos)
        continue;

      auto & lm_sys = problem->_solver_systems[i]->system();
      for (auto & [mat_name, mat] : as_range(lm_sys.matrices_begin(), lm_sys.matrices_end()))
      {
        libmesh_ignore(mat_name);
        auto * const petsc_mat = cast_ptr<PetscMatrix<Number> *>(mat.get());
        LibmeshPetscCall2(comm, MatSetFromOptions(petsc_mat->mat()));
      }
      found_matching_prefix = true;
      break;
    }

    if (!found_matching_prefix)
      mooseError("We did not find a matching solver system name for the petsc option '",
                 lower_case_name,
                 "'");
  }
}

void
colorAdjacencyMatrix(PetscScalar * adjacency_matrix,
                     unsigned int size,
                     unsigned int colors,
                     std::vector<unsigned int> & vertex_colors,
                     const char * coloring_algorithm)
{
  // Mat A will be a dense matrix from the incoming data structure
  Mat A;
  LibmeshPetscCallA(PETSC_COMM_SELF, MatCreate(PETSC_COMM_SELF, &A));
  LibmeshPetscCallA(PETSC_COMM_SELF, MatSetSizes(A, size, size, size, size));
  LibmeshPetscCallA(PETSC_COMM_SELF, MatSetType(A, MATSEQDENSE));
  // PETSc requires a non-const data array to populate the matrix
  LibmeshPetscCallA(PETSC_COMM_SELF, MatSeqDenseSetPreallocation(A, adjacency_matrix));
  LibmeshPetscCallA(PETSC_COMM_SELF, MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY));
  LibmeshPetscCallA(PETSC_COMM_SELF, MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY));

  // Convert A to a sparse matrix
#if PETSC_VERSION_LESS_THAN(3, 7, 0)
  LibmeshPetscCallA(PETSC_COMM_SELF, MatConvert(A, MATAIJ, MAT_REUSE_MATRIX, &A));
#else
  LibmeshPetscCallA(PETSC_COMM_SELF, MatConvert(A, MATAIJ, MAT_INPLACE_MATRIX, &A));
#endif

  ISColoring iscoloring;
  MatColoring mc;
  LibmeshPetscCallA(PETSC_COMM_SELF, MatColoringCreate(A, &mc));
  LibmeshPetscCallA(PETSC_COMM_SELF, MatColoringSetType(mc, coloring_algorithm));
  LibmeshPetscCallA(PETSC_COMM_SELF, MatColoringSetMaxColors(mc, static_cast<PetscInt>(colors)));

  // Petsc normally colors by distance two (neighbors of neighbors), we just want one
  LibmeshPetscCallA(PETSC_COMM_SELF, MatColoringSetDistance(mc, 1));
  LibmeshPetscCallA(PETSC_COMM_SELF, MatColoringSetFromOptions(mc));
  LibmeshPetscCallA(PETSC_COMM_SELF, MatColoringApply(mc, &iscoloring));

  PetscInt nn;
  IS * is;
#if PETSC_RELEASE_LESS_THAN(3, 12, 0)
  LibmeshPetscCallA(PETSC_COMM_SELF, ISColoringGetIS(iscoloring, &nn, &is));
#else
  LibmeshPetscCallA(PETSC_COMM_SELF, ISColoringGetIS(iscoloring, PETSC_USE_POINTER, &nn, &is));
#endif

  if (nn > static_cast<PetscInt>(colors))
    throw std::runtime_error("Not able to color with designated number of colors");

  for (int i = 0; i < nn; i++)
  {
    PetscInt isize;
    const PetscInt * indices;
    LibmeshPetscCallA(PETSC_COMM_SELF, ISGetLocalSize(is[i], &isize));
    LibmeshPetscCallA(PETSC_COMM_SELF, ISGetIndices(is[i], &indices));
    for (int j = 0; j < isize; j++)
    {
      mooseAssert(indices[j] < static_cast<PetscInt>(vertex_colors.size()), "Index out of bounds");
      vertex_colors[indices[j]] = i;
    }
    LibmeshPetscCallA(PETSC_COMM_SELF, ISRestoreIndices(is[i], &indices));
  }

  LibmeshPetscCallA(PETSC_COMM_SELF, MatDestroy(&A));
  LibmeshPetscCallA(PETSC_COMM_SELF, MatColoringDestroy(&mc));
  LibmeshPetscCallA(PETSC_COMM_SELF, ISColoringDestroy(&iscoloring));
}

void
disableNonlinearConvergedReason(FEProblemBase & fe_problem)
{
  auto & petsc_options = fe_problem.getPetscOptions();

  petsc_options.flags.eraseSetValue("-snes_converged_reason");

  auto & pairs = petsc_options.pairs;
  auto it = MooseUtils::findPair(pairs, "-snes_converged_reason", MooseUtils::Any);
  if (it != pairs.end())
    pairs.erase(it);
}

void
disableLinearConvergedReason(FEProblemBase & fe_problem)
{
  auto & petsc_options = fe_problem.getPetscOptions();

  petsc_options.flags.eraseSetValue("-ksp_converged_reason");

  auto & pairs = petsc_options.pairs;
  auto it = MooseUtils::findPair(pairs, "-ksp_converged_reason", MooseUtils::Any);
  if (it != pairs.end())
    pairs.erase(it);
}

} // Namespace PetscSupport
} // Namespace MOOSE
