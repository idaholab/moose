//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PetscSupport.h"

#ifdef LIBMESH_HAVE_PETSC

// MOOSE includes
#include "MooseApp.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "NonlinearSystem.h"
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

#include "libmesh/equation_systems.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/petsc_linear_solver.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/petsc_preconditioner.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/sparse_matrix.h"

// PETSc includes
#include <petsc.h>
#include <petscsnes.h>
#include <petscksp.h>

// For graph coloring
#include <petscmat.h>
#include <petscis.h>

#if PETSC_VERSION_LESS_THAN(3, 3, 0)
// PETSc 3.2.x and lower
#include <private/kspimpl.h>
#include <private/snesimpl.h>
#else
// PETSc 3.3.0+
#include <petscdm.h>
#endif

// PetscDMMoose include
#include "PetscDMMoose.h"

// Standard includes
#include <ostream>
#include <fstream>
#include <string>

void
MooseVecView(NumericVector<Number> & vector)
{
  PetscVector<Number> & petsc_vec = static_cast<PetscVector<Number> &>(vector);
  VecView(petsc_vec.vec(), 0);
}

void
MooseMatView(SparseMatrix<Number> & mat)
{
  PetscMatrix<Number> & petsc_mat = static_cast<PetscMatrix<Number> &>(mat);
  MatView(petsc_mat.mat(), 0);
}

void
MooseVecView(const NumericVector<Number> & vector)
{
  PetscVector<Number> & petsc_vec =
      static_cast<PetscVector<Number> &>(const_cast<NumericVector<Number> &>(vector));
  VecView(petsc_vec.vec(), 0);
}

void
MooseMatView(const SparseMatrix<Number> & mat)
{
  PetscMatrix<Number> & petsc_mat =
      static_cast<PetscMatrix<Number> &>(const_cast<SparseMatrix<Number> &>(mat));
  MatView(petsc_mat.mat(), 0);
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
#if PETSC_VERSION_LESS_THAN(3, 3, 0)
    case LS_CUBIC:
      return "cubic";
    case LS_QUADRATIC:
      return "quadratic";
    case LS_BASICNONORMS:
      return "basicnonorms";
#else
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
#endif
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
setSolverOptions(SolverParams & solver_params)
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
  {
#if PETSC_VERSION_LESS_THAN(3, 3, 0)
    setSinglePetscOption("-snes_type", "ls");
    setSinglePetscOption("-snes_ls", stringify(ls_type));
#else
    setSinglePetscOption("-snes_linesearch_type", stringify(ls_type));
#endif
  }
}

void
petscSetupDM(NonlinearSystemBase & nl)
{
#if !PETSC_VERSION_LESS_THAN(3, 3, 0)
  PetscErrorCode ierr;
  PetscBool ismoose;
  DM dm = PETSC_NULL;

  // Initialize the part of the DM package that's packaged with Moose; in the PETSc source tree this
  // call would be in DMInitializePackage()
  ierr = DMMooseRegisterAll();
  CHKERRABORT(nl.comm().get(), ierr);
  // Create and set up the DM that will consume the split options and deal with block matrices.
  PetscNonlinearSolver<Number> * petsc_solver =
      dynamic_cast<PetscNonlinearSolver<Number> *>(nl.nonlinearSolver());
  SNES snes = petsc_solver->snes();
  // if there exists a DMMoose object, not to recreate a new one
  ierr = SNESGetDM(snes, &dm);
  CHKERRABORT(nl.comm().get(), ierr);
  if (dm)
  {
    ierr = PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose);
    CHKERRABORT(nl.comm().get(), ierr);
    if (ismoose)
      return;
  }
  ierr = DMCreateMoose(nl.comm().get(), nl, &dm);
  CHKERRABORT(nl.comm().get(), ierr);
  ierr = DMSetFromOptions(dm);
  CHKERRABORT(nl.comm().get(), ierr);
  ierr = DMSetUp(dm);
  CHKERRABORT(nl.comm().get(), ierr);
  ierr = SNESSetDM(snes, dm);
  CHKERRABORT(nl.comm().get(), ierr);
  ierr = DMDestroy(&dm);
  CHKERRABORT(nl.comm().get(), ierr);
// We temporarily comment out this updating function because
// we lack an approach to check if the problem
// structure has been changed from the last iteration.
// The indices will be rebuilt for every timestep.
// TODO: figure out a way to check the structure changes of the
// matrix
// ierr = SNESSetUpdate(snes,SNESUpdateDMMoose);
// CHKERRABORT(nl.comm().get(),ierr);
#endif
}

void
addPetscOptionsFromCommandline()
{
  // commandline options always win
  // the options from a user commandline will overwrite the existing ones if any conflicts
  { // Get any options specified on the command-line
    int argc;
    char ** args;

    PetscGetArgs(&argc, &args);
#if PETSC_VERSION_LESS_THAN(3, 7, 0)
    PetscOptionsInsert(&argc, &args, NULL);
#else
    PetscOptionsInsert(PETSC_NULL, &argc, &args, NULL);
#endif
  }
}

void
petscSetOptions(FEProblemBase & problem)
{
  // Reference to the options stored in FEPRoblem
  PetscOptions & petsc = problem.getPetscOptions();

  if (petsc.inames.size() != petsc.values.size())
    mooseError("PETSc names and options are not the same length");

#if PETSC_VERSION_LESS_THAN(3, 7, 0)
  PetscOptionsClear();
#else
  PetscOptionsClear(PETSC_NULL);
#endif

  setSolverOptions(problem.solverParams());

  // Add any additional options specified in the input file
  for (const auto & flag : petsc.flags)
    setSinglePetscOption(flag.rawName().c_str());
  // Add option pairs
  for (unsigned int i = 0; i < petsc.inames.size(); ++i)
    setSinglePetscOption(petsc.inames[i], petsc.values[i]);

  addPetscOptionsFromCommandline();
}

PetscErrorCode
petscSetupOutput(CommandLine * cmd_line)
{
  char code[10] = {45, 45, 109, 111, 111, 115, 101};
  const std::vector<std::string> argv = cmd_line->getArguments();
  for (const auto & arg : argv)
  {
    if (arg == std::string(code, 10))
    {
      Console::petscSetupOutput();
      break;
    }
  }
  return 0;
}

PetscErrorCode
petscNonlinearConverged(SNES snes,
                        PetscInt it,
                        PetscReal xnorm,
                        PetscReal snorm,
                        PetscReal fnorm,
                        SNESConvergedReason * reason,
                        void * ctx)
{
  FEProblemBase & problem = *static_cast<FEProblemBase *>(ctx);
  NonlinearSystemBase & system = problem.getNonlinearSystemBase();

  // Let's be nice and always check PETSc error codes.
  PetscErrorCode ierr = 0;

  // Temporary variables to store SNES tolerances.  Usual C-style would be to declare
  // but not initialize these... but it bothers me to leave anything uninitialized.
  PetscReal atol = 0.; // absolute convergence tolerance
  PetscReal rtol = 0.; // relative convergence tolerance
  PetscReal stol = 0.; // convergence (step) tolerance in terms of the norm of the change in the
                       // solution between steps
  PetscInt maxit = 0;  // maximum number of iterations
  PetscInt maxf = 0;   // maximum number of function evaluations

  // Ask the SNES object about its tolerances.
  ierr = SNESGetTolerances(snes, &atol, &rtol, &stol, &maxit, &maxf);
  CHKERRABORT(problem.comm().get(), ierr);

  // Ask the SNES object about its divergence tolerance.
  PetscReal divtol = 0.; // relative divergence tolerance
#if !PETSC_VERSION_LESS_THAN(3, 8, 0)
  ierr = SNESGetDivergenceTolerance(snes, &divtol);
  CHKERRABORT(problem.comm().get(), ierr);
#endif

  // Get current number of function evaluations done by SNES.
  PetscInt nfuncs = 0;
  ierr = SNESGetNumberFunctionEvals(snes, &nfuncs);
  CHKERRABORT(problem.comm().get(), ierr);

  // Whether or not to force SNESSolve() take at least one iteration regardless of the initial
  // residual norm
  PetscBool force_iteration = PETSC_FALSE;
#if !PETSC_VERSION_LESS_THAN(3, 8, 4)
  ierr = SNESGetForceIteration(snes, &force_iteration);
  CHKERRABORT(problem.comm().get(), ierr);
#endif

// See if SNESSetFunctionDomainError() has been called.  Note:
// SNESSetFunctionDomainError() and SNESGetFunctionDomainError()
// were added in different releases of PETSc.
#if !PETSC_VERSION_LESS_THAN(3, 3, 0)
  PetscBool domainerror;
  ierr = SNESGetFunctionDomainError(snes, &domainerror);
  CHKERRABORT(problem.comm().get(), ierr);
  if (domainerror)
  {
    *reason = SNES_DIVERGED_FUNCTION_DOMAIN;
    return 0;
  }
#endif

  // Error message that will be set by the FEProblemBase.
  std::string msg;

  // xnorm: 2-norm of current iterate
  // snorm: 2-norm of current step
  // fnorm: 2-norm of function at current iterate
  MooseNonlinearConvergenceReason moose_reason =
      problem.checkNonlinearConvergence(msg,
                                        it,
                                        xnorm,
                                        snorm,
                                        fnorm,
                                        rtol,
                                        divtol,
                                        stol,
                                        atol,
                                        nfuncs,
                                        maxf,
                                        force_iteration,
                                        system._initial_residual_before_preset_bcs,
                                        std::numeric_limits<Real>::max());

  if (msg.length() > 0)
    PetscInfo(snes, msg.c_str());

  switch (moose_reason)
  {
    case MooseNonlinearConvergenceReason::ITERATING:
      *reason = SNES_CONVERGED_ITERATING;
      break;

    case MooseNonlinearConvergenceReason::CONVERGED_FNORM_ABS:
      *reason = SNES_CONVERGED_FNORM_ABS;
      break;

    case MooseNonlinearConvergenceReason::CONVERGED_FNORM_RELATIVE:
      *reason = SNES_CONVERGED_FNORM_RELATIVE;
      break;

    case MooseNonlinearConvergenceReason::DIVERGED_DTOL:
#if !PETSC_VERSION_LESS_THAN(3, 8, 0) // A new convergence enum in PETSc 3.8
      *reason = SNES_DIVERGED_DTOL;
#endif
      break;

    case MooseNonlinearConvergenceReason::CONVERGED_SNORM_RELATIVE:
#if PETSC_VERSION_LESS_THAN(3, 3, 0)
      *reason = SNES_CONVERGED_PNORM_RELATIVE;
#else
      *reason = SNES_CONVERGED_SNORM_RELATIVE;
#endif
      break;

    case MooseNonlinearConvergenceReason::DIVERGED_FUNCTION_COUNT:
      *reason = SNES_DIVERGED_FUNCTION_COUNT;
      break;

    case MooseNonlinearConvergenceReason::DIVERGED_FNORM_NAN:
      *reason = SNES_DIVERGED_FNORM_NAN;
      break;

    case MooseNonlinearConvergenceReason::DIVERGED_LINE_SEARCH:
#if PETSC_VERSION_LESS_THAN(3, 2, 0)
      *reason = SNES_DIVERGED_LS_FAILURE;
#else
      *reason = SNES_DIVERGED_LINE_SEARCH;
#endif
      break;
  }

  return 0;
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
  NonlinearSystemBase & nl = problem.getNonlinearSystemBase();

  KSPSetNormType(ksp, getPetscKSPNormType(nl.getMooseKSPNormType()));
}

void
petscSetDefaultPCSide(FEProblemBase & problem, KSP ksp)
{
  NonlinearSystemBase & nl = problem.getNonlinearSystemBase();

#if PETSC_VERSION_LESS_THAN(3, 2, 0)
  // pc_side is NOT set, PETSc will make the decision
  // PETSc 3.1.x-
  if (nl.getPCSide() != Moose::PCS_DEFAULT)
    KSPSetPreconditionerSide(ksp, getPetscPCSide(nl.getPCSide()));
#else
  // PETSc 3.2.x+
  if (nl.getPCSide() != Moose::PCS_DEFAULT)
    KSPSetPCSide(ksp, getPetscPCSide(nl.getPCSide()));
#endif
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
  KSPSetTolerances(ksp, rtol, atol, 1e100, maxits);

  petscSetDefaultPCSide(problem, ksp);

  petscSetDefaultKSPNormType(problem, ksp);
}

void
petscSetDefaults(FEProblemBase & problem)
{
  // dig out Petsc solver
  NonlinearSystemBase & nl = problem.getNonlinearSystemBase();
  PetscNonlinearSolver<Number> * petsc_solver =
      dynamic_cast<PetscNonlinearSolver<Number> *>(nl.nonlinearSolver());
  SNES snes = petsc_solver->snes();
  KSP ksp;
  SNESGetKSP(snes, &ksp);

  SNESSetMaxLinearSolveFailures(snes, 1000000);

#if PETSC_VERSION_LESS_THAN(3, 0, 0)
  // PETSc 2.3.3-
  SNESSetConvergenceTest(snes, petscNonlinearConverged, &problem);
#else
  // PETSc 3.0.0+

  // In 3.0.0, the context pointer must actually be used, and the
  // final argument to KSPSetConvergenceTest() is a pointer to a
  // routine for destroying said private data context.  In this case,
  // we use the default context provided by PETSc in addition to
  // a few other tests.
  {
    auto ierr = SNESSetConvergenceTest(snes, petscNonlinearConverged, &problem, PETSC_NULL);
    CHKERRABORT(nl.comm().get(), ierr);
  }
#endif

  petscSetKSPDefaults(problem, ksp);
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
      {
        NonlinearImplicitSystem * nl_system =
            dynamic_cast<NonlinearImplicitSystem *>(&fe_problem.getNonlinearSystemBase().system());
        if (!nl_system)
          mooseError("You've requested a line search but you must be solving an EigenProblem. "
                     "These two things are not consistent.");
        PetscNonlinearSolver<Real> * petsc_nonlinear_solver =
            dynamic_cast<PetscNonlinearSolver<Real> *>(nl_system->nonlinear_solver.get());
        if (!petsc_nonlinear_solver)
          mooseError("Currently the MOOSE line searches all use Petsc, so you "
                     "must use Petsc as your non-linear solver.");
        petsc_nonlinear_solver->linesearch_object =
            libmesh_make_unique<ComputeLineSearchObjectWrapper>(fe_problem);
      }
    }
  }

  if (params.isParamValid("mffd_type"))
  {
    MooseEnum mffd_type = params.get<MooseEnum>("mffd_type");
    fe_problem.solverParams()._mffd_type = Moose::stringToEnum<Moose::MffdType>(mffd_type);
  }

  // The parameters contained in the Action
  const MultiMooseEnum & petsc_options = params.get<MultiMooseEnum>("petsc_options");
  const MultiMooseEnum & petsc_options_inames = params.get<MultiMooseEnum>("petsc_options_iname");
  const std::vector<std::string> & petsc_options_values =
      params.get<std::vector<std::string>>("petsc_options_value");

  // A reference to the PetscOptions object that contains the settings that will be used in the
  // solve
  Moose::PetscSupport::PetscOptions & po = fe_problem.getPetscOptions();

  // Update the PETSc single flags
  for (const auto & option : petsc_options)
  {
    /**
     * "-log_summary" cannot be used in the input file. This option needs to be set when PETSc is
     * initialized
     * which happens before the parser is even created.  We'll throw an error if somebody attempts
     * to add this option later.
     */
    if (option == "-log_summary")
      mooseError("The PETSc option \"-log_summary\" can only be used on the command line.  Please "
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
    if (!po.flags.contains(option))
      po.flags.push_back(option);
  }

  std::array<std::string, 2> reason_flags = {{"-snes_converged_reason", "-ksp_converged_reason"}};

  for (const auto & reason_flag : reason_flags)
    if (!po.flags.contains(reason_flag))
      po.flags.push_back(reason_flag);

  // Check that the name value pairs are sized correctly
  if (petsc_options_inames.size() != petsc_options_values.size())
    mooseError("PETSc names and options are not the same length");

  // Setup the name value pairs
  bool boomeramg_found = false;
  bool strong_threshold_found = false;
#if !PETSC_VERSION_LESS_THAN(3, 7, 0)
  bool superlu_dist_found = false;
  bool fact_pattern_found = false;
  bool tiny_pivot_found = false;
#endif
  std::string pc_description = "";
  for (unsigned int i = 0; i < petsc_options_inames.size(); i++)
  {
    // Do not add duplicate settings
    if (find(po.inames.begin(), po.inames.end(), petsc_options_inames[i]) == po.inames.end())
    {
#if !PETSC_VERSION_LESS_THAN(3, 9, 0)
      if (petsc_options_inames[i] == "-pc_factor_mat_solver_package")
        po.inames.push_back("-pc_factor_mat_solver_type");
      else
        po.inames.push_back(petsc_options_inames[i]);
#else
      if (petsc_options_inames[i] == "-pc_factor_mat_solver_type")
        po.inames.push_back("-pc_factor_mat_solver_package");
      else
        po.inames.push_back(petsc_options_inames[i]);
#endif
      po.values.push_back(petsc_options_values[i]);

      // Look for a pc description
      if (petsc_options_inames[i] == "-pc_type" || petsc_options_inames[i] == "-pc_sub_type" ||
          petsc_options_inames[i] == "-pc_hypre_type")
        pc_description += petsc_options_values[i] + ' ';

      // This special case is common enough that we'd like to handle it for the user.
      if (petsc_options_inames[i] == "-pc_hypre_type" && petsc_options_values[i] == "boomeramg")
        boomeramg_found = true;
      if (petsc_options_inames[i] == "-pc_hypre_boomeramg_strong_threshold")
        strong_threshold_found = true;
#if !PETSC_VERSION_LESS_THAN(3, 7, 0)
      if ((petsc_options_inames[i] == "-pc_factor_mat_solver_package" ||
           petsc_options_inames[i] == "-pc_factor_mat_solver_type") &&
          petsc_options_values[i] == "superlu_dist")
        superlu_dist_found = true;
      if (petsc_options_inames[i] == "-mat_superlu_dist_fact")
        fact_pattern_found = true;
      if (petsc_options_inames[i] == "-mat_superlu_dist_replacetinypivot")
        tiny_pivot_found = true;
#endif
    }
    else
    {
      for (unsigned int j = 0; j < po.inames.size(); j++)
        if (po.inames[j] == petsc_options_inames[i])
          po.values[j] = petsc_options_values[i];
    }
  }

  // When running a 3D mesh with boomeramg, it is almost always best to supply a strong threshold
  // value
  // We will provide that for the user here if they haven't supplied it themselves.
  if (boomeramg_found && !strong_threshold_found && fe_problem.mesh().dimension() == 3)
  {
    po.inames.push_back("-pc_hypre_boomeramg_strong_threshold");
    po.values.push_back("0.7");
    pc_description += "strong_threshold: 0.7 (auto)";
  }

#if !PETSC_VERSION_LESS_THAN(3, 7, 0)
  // In PETSc-3.7.{0--4}, there is a bug when using superlu_dist, and we have to use
  // SamePattern_SameRowPerm, otherwise we use whatever we have in PETSc
  if (superlu_dist_found && !fact_pattern_found)
  {
    po.inames.push_back("-mat_superlu_dist_fact");
#if PETSC_VERSION_LESS_THAN(3, 7, 5)
    po.values.push_back("SamePattern_SameRowPerm");
    pc_description += "mat_superlu_dist_fact: SamePattern_SameRowPerm ";
#else
    po.values.push_back("SamePattern");
    pc_description += "mat_superlu_dist_fact: SamePattern ";
#endif
  }

  // restore this superlu  option
  if (superlu_dist_found && !tiny_pivot_found)
  {
    po.inames.push_back("-mat_superlu_dist_replacetinypivot");
    po.values.push_back("1");
    pc_description += " mat_superlu_dist_replacetinypivot: true ";
  }
#endif
  // Set Preconditioner description
  po.pc_description = pc_description;
}

std::set<std::string>
getPetscValidLineSearches()
{
#if PETSC_VERSION_LESS_THAN(3, 3, 0)
  return {"default", "cubic", "quadratic", "none", "basic", "basicnonorms"};
#else
  return {"default", "shell", "none", "basic", "l2", "bt", "cp"};
#endif
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
  PetscGetArgs(&argc, &args);

  std::vector<std::string> cml_arg;
  for (int i = 0; i < argc; i++)
    cml_arg.push_back(args[i]);

  if (std::find(petsc.values.begin(), petsc.values.end(), "vinewtonssls") == petsc.values.end() &&
      std::find(petsc.values.begin(), petsc.values.end(), "vinewtonrsls") == petsc.values.end() &&
      std::find(cml_arg.begin(), cml_arg.end(), "vinewtonssls") == cml_arg.end() &&
      std::find(cml_arg.begin(), cml_arg.end(), "vinewtonrsls") == cml_arg.end())
    return false;

  return true;
}

void
setSinglePetscOption(const std::string & name, const std::string & value)
{
  PetscErrorCode ierr;

#if PETSC_VERSION_LESS_THAN(3, 7, 0)
  ierr = PetscOptionsSetValue(name.c_str(), value == "" ? PETSC_NULL : value.c_str());
#else
  // PETSc 3.7.0 and later version.  First argument is the options
  // database to use, NULL indicates the default global database.
  ierr = PetscOptionsSetValue(PETSC_NULL, name.c_str(), value == "" ? PETSC_NULL : value.c_str());
#endif

  // Not convenient to use the usual error checking macro, because we
  // don't have a specific communicator in this helper function.
  if (ierr)
    mooseError("Error setting PETSc option: ", name);
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
  MatCreate(MPI_COMM_SELF, &A);
  MatSetSizes(A, size, size, size, size);
  MatSetType(A, MATSEQDENSE);
  // PETSc requires a non-const data array to populate the matrix
  MatSeqDenseSetPreallocation(A, adjacency_matrix);
  MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);
  MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);

  // Convert A to a sparse matrix
  MatConvert(A,
             MATAIJ,
#if PETSC_VERSION_LESS_THAN(3, 7, 0)
             MAT_REUSE_MATRIX,
#else
             MAT_INPLACE_MATRIX,
#endif
             &A);

  ISColoring iscoloring;
#if PETSC_VERSION_LESS_THAN(3, 5, 0)
  MatGetColoring(A, coloring_algorithm, &iscoloring);
#else
  MatColoring mc;
  MatColoringCreate(A, &mc);
  MatColoringSetType(mc, coloring_algorithm);
  MatColoringSetMaxColors(mc, static_cast<PetscInt>(colors));

  // Petsc normally colors by distance two (neighbors of neighbors), we just want one
  MatColoringSetDistance(mc, 1);
  MatColoringSetFromOptions(mc);
  MatColoringApply(mc, &iscoloring);
#endif

  PetscInt nn;
  IS * is;
#if PETSC_RELEASE_LESS_THAN(3, 12, 0)
  ISColoringGetIS(iscoloring, &nn, &is);
#else
  ISColoringGetIS(iscoloring, PETSC_USE_POINTER, &nn, &is);
#endif

  if (nn > static_cast<PetscInt>(colors))
    throw std::runtime_error("Not able to color with designated number of colors");

  for (int i = 0; i < nn; i++)
  {
    PetscInt isize;
    const PetscInt * indices;
    ISGetLocalSize(is[i], &isize);
    ISGetIndices(is[i], &indices);
    for (int j = 0; j < isize; j++)
    {
      mooseAssert(indices[j] < static_cast<PetscInt>(vertex_colors.size()), "Index out of bounds");
      vertex_colors[indices[j]] = i;
    }
    ISRestoreIndices(is[i], &indices);
  }

  MatDestroy(&A);
#if !PETSC_VERSION_LESS_THAN(3, 5, 0)
  MatColoringDestroy(&mc);
#endif
  ISColoringDestroy(&iscoloring);
}

void
disableNonlinearConvergedReason(FEProblemBase & fe_problem)
{
  auto & petsc_options = fe_problem.getPetscOptions();

  petsc_options.flags.erase("-snes_converged_reason");
}

void
disableLinearConvergedReason(FEProblemBase & fe_problem)
{
  auto & petsc_options = fe_problem.getPetscOptions();

  petsc_options.flags.erase("-ksp_converged_reason");
}

} // Namespace PetscSupport
} // Namespace MOOSE

#endif // LIBMESH_HAVE_PETSC
