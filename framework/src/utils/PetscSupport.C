/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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

// libMesh includes
#include "libmesh/libmesh_common.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_linear_solver.h"
#include "libmesh/petsc_preconditioner.h"
#include "libmesh/getpot.h"

// PETSc includes
#include <petsc.h>
#include <petscsnes.h>
#include <petscksp.h>

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
#endif
    case LS_INVALID:
      mooseError("Invalid LineSearchType");
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
      break;

    case Moose::ST_JFNK:
      setSinglePetscOption("-snes_mf");
      break;

    case Moose::ST_NEWTON:
      break;

    case Moose::ST_FD:
      setSinglePetscOption("-snes_fd");
      break;

    case Moose::ST_LINEAR:
      setSinglePetscOption("-snes_type", "ksponly");
      break;
  }

  Moose::LineSearchType ls_type = solver_params._line_search;
  if (ls_type == Moose::LS_NONE)
    ls_type = Moose::LS_BASIC;

  if (ls_type != Moose::LS_DEFAULT)
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
    setSinglePetscOption(flag.c_str());
  for (unsigned int i = 0; i < petsc.inames.size(); ++i)
    setSinglePetscOption(petsc.inames[i], petsc.values[i]);

  // set up DM which is required if use a field split preconditioner
  if (problem.getNonlinearSystemBase().haveFieldSplitPreconditioner())
    petscSetupDM(problem.getNonlinearSystemBase());

  addPetscOptionsFromCommandline();
}

PetscErrorCode
petscSetupOutput(CommandLine * cmd_line)
{
  char code[10] = {45, 45, 109, 111, 111, 115, 101};
  if (cmd_line->getPot()->search(code))
    Console::petscSetupOutput();
  return 0;
}

PetscErrorCode
petscConverged(KSP ksp, PetscInt n, PetscReal rnorm, KSPConvergedReason * reason, void * ctx)
{
  // Cast the context pointer coming from PETSc to an FEProblemBase& and
  // get a reference to the System from it.
  FEProblemBase & problem = *static_cast<FEProblemBase *>(ctx);

  // Let's be nice and always check PETSc error codes.
  PetscErrorCode ierr = 0;

  // We want the default behavior of the KSPDefaultConverged test, but
  // we don't want PETSc to die in that function with a CHKERRQ
  // call... that is probably extremely unlikely/impossible, but just
  // to be on the safe side, we push a different error handler before
  // calling KSPDefaultConverged().
  ierr = PetscPushErrorHandler(PetscReturnErrorHandler, /*void* ctx=*/PETSC_NULL);
  CHKERRABORT(problem.comm().get(), ierr);

#if PETSC_VERSION_LESS_THAN(3, 0, 0)
  // Prior to PETSc 3.0.0, you could call KSPDefaultConverged with a NULL context
  // pointer, as it was unused.
  KSPDefaultConverged(ksp, n, rnorm, reason, PETSC_NULL);
#elif PETSC_RELEASE_LESS_THAN(3, 5, 0)
  // As of PETSc 3.0.0, you must call KSPDefaultConverged with a
  // non-NULL context pointer which must be created with
  // KSPDefaultConvergedCreate(), and destroyed with
  // KSPDefaultConvergedDestroy().
  void * default_ctx = NULL;
  KSPDefaultConvergedCreate(&default_ctx);
  KSPDefaultConverged(ksp, n, rnorm, reason, default_ctx);
  KSPDefaultConvergedDestroy(default_ctx);
#else
  // As of PETSc 3.5.0, use KSPConvergedDefaultXXX
  void * default_ctx = NULL;
  KSPConvergedDefaultCreate(&default_ctx);
  KSPConvergedDefault(ksp, n, rnorm, reason, default_ctx);
  KSPConvergedDefaultDestroy(default_ctx);
#endif

  // Pop the Error handler we pushed on the stack to go back
  // to default PETSc error handling behavior.
  ierr = PetscPopErrorHandler();
  CHKERRABORT(problem.comm().get(), ierr);

  // Get tolerances from the KSP object
  PetscReal rtol = 0.;
  PetscReal atol = 0.;
  PetscReal dtol = 0.;
  PetscInt maxits = 0;
  ierr = KSPGetTolerances(ksp, &rtol, &atol, &dtol, &maxits);
  CHKERRABORT(problem.comm().get(), ierr);

  // Now do some additional MOOSE-specific tests...
  std::string msg;
  MooseLinearConvergenceReason moose_reason =
      problem.checkLinearConvergence(msg, n, rnorm, rtol, atol, dtol, maxits);

  switch (moose_reason)
  {
    case MOOSE_CONVERGED_RTOL:
      *reason = KSP_CONVERGED_RTOL;
      break;

    case MOOSE_CONVERGED_ITS:
      *reason = KSP_CONVERGED_ITS;
      break;

    case MOOSE_DIVERGED_NANORINF:
#if PETSC_VERSION_LESS_THAN(3, 4, 0)
      // Report divergence due to exceeding the divergence tolerance.
      *reason = KSP_DIVERGED_DTOL;
#else
      // KSP_DIVERGED_NANORINF was added in PETSc 3.4.0.
      *reason = KSP_DIVERGED_NANORINF;
#endif
      break;
#if !PETSC_VERSION_LESS_THAN(3, 6, 0) // A new convergence enum in PETSc 3.6
    case MOOSE_DIVERGED_PCSETUP_FAILED:
      *reason = KSP_DIVERGED_PCSETUP_FAILED;
      break;
#endif
    default:
    {
      // If it's not either of the two specific cases we handle, just go
      // with whatever PETSc decided in KSPDefaultConverged.
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

  // Get current number of function evaluations done by SNES.
  PetscInt nfuncs = 0;
  ierr = SNESGetNumberFunctionEvals(snes, &nfuncs);
  CHKERRABORT(problem.comm().get(), ierr);

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
  MooseNonlinearConvergenceReason moose_reason = problem.checkNonlinearConvergence(
      msg,
      it,
      xnorm,
      snorm,
      fnorm,
      rtol,
      stol,
      atol,
      nfuncs,
      maxf,
      system._initial_residual_before_preset_bcs,
      /*div_threshold=*/(1.0 / rtol) * system._initial_residual_before_preset_bcs);

  if (msg.length() > 0)
    PetscInfo(snes, msg.c_str());

  switch (moose_reason)
  {
    case MOOSE_NONLINEAR_ITERATING:
      *reason = SNES_CONVERGED_ITERATING;
      break;

    case MOOSE_CONVERGED_FNORM_ABS:
      *reason = SNES_CONVERGED_FNORM_ABS;
      break;

    case MOOSE_CONVERGED_FNORM_RELATIVE:
      *reason = SNES_CONVERGED_FNORM_RELATIVE;
      break;

    case MOOSE_CONVERGED_SNORM_RELATIVE:
#if PETSC_VERSION_LESS_THAN(3, 3, 0)
      *reason = SNES_CONVERGED_PNORM_RELATIVE;
#else
      *reason = SNES_CONVERGED_SNORM_RELATIVE;
#endif
      break;

    case MOOSE_DIVERGED_FUNCTION_COUNT:
      *reason = SNES_DIVERGED_FUNCTION_COUNT;
      break;

    case MOOSE_DIVERGED_FNORM_NAN:
      *reason = SNES_DIVERGED_FNORM_NAN;
      break;

    case MOOSE_DIVERGED_LINE_SEARCH:
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
  PCSide pcside = getPetscPCSide(nl.getPCSide());
#if PETSC_VERSION_LESS_THAN(3, 2, 0)
  // PETSc 3.1.x-
  KSPSetPreconditionerSide(ksp, pcside);
#else
  // PETSc 3.2.x+
  KSPSetPCSide(ksp, pcside);
#endif
  SNESSetMaxLinearSolveFailures(snes, 1000000);

#if PETSC_VERSION_LESS_THAN(3, 0, 0)
  // PETSc 2.3.3-
  KSPSetConvergenceTest(ksp, petscConverged, &problem);
  SNESSetConvergenceTest(snes, petscNonlinearConverged, &problem);
#else
  // PETSc 3.0.0+

  // In 3.0.0, the context pointer must actually be used, and the
  // final argument to KSPSetConvergenceTest() is a pointer to a
  // routine for destroying said private data context.  In this case,
  // we use the default context provided by PETSc in addition to
  // a few other tests.
  {
    PetscErrorCode ierr = KSPSetConvergenceTest(ksp, petscConverged, &problem, PETSC_NULL);
    CHKERRABORT(nl.comm().get(), ierr);
    ierr = SNESSetConvergenceTest(snes, petscNonlinearConverged, &problem, PETSC_NULL);
    CHKERRABORT(nl.comm().get(), ierr);
  }
#endif
}

void
storePetscOptions(FEProblemBase & fe_problem, const InputParameters & params)
{
  // Note: Options set in the Preconditioner block will override those set in the Executioner block
  if (params.isParamValid("solve_type"))
  {
    // Extract the solve type
    const std::string & solve_type = params.get<MooseEnum>("solve_type");
    fe_problem.solverParams()._type = Moose::stringToEnum<Moose::SolveType>(solve_type);
  }

  if (params.isParamValid("line_search"))
  {
    MooseEnum line_search = params.get<MooseEnum>("line_search");
    if (fe_problem.solverParams()._line_search == Moose::LS_INVALID || line_search != "default")
      fe_problem.solverParams()._line_search =
          Moose::stringToEnum<Moose::LineSearchType>(line_search);
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
                     option,
                     " should not be used directly in a MOOSE input file. ",
                     help_string);
    }

    // Update the stored items, but do not create duplicates
    if (find(po.flags.begin(), po.flags.end(), option) == po.flags.end())
      po.flags.push_back(option);
  }

  // Check that the name value pairs are sized correctly
  if (petsc_options_inames.size() != petsc_options_values.size())
    mooseError("PETSc names and options are not the same length");

  // Setup the name value pairs
  bool boomeramg_found = false;
  bool strong_threshold_found = false;
#if !PETSC_VERSION_LESS_THAN(3, 7, 0)
  bool superlu_dist_found = false;
  bool fact_pattern_found = false;
#endif
  std::string pc_description = "";
  for (unsigned int i = 0; i < petsc_options_inames.size(); i++)
  {
    // Do not add duplicate settings
    if (find(po.inames.begin(), po.inames.end(), petsc_options_inames[i]) == po.inames.end())
    {
      po.inames.push_back(petsc_options_inames[i]);
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
      if (petsc_options_inames[i] == "-pc_factor_mat_solver_package" &&
          petsc_options_values[i] == "superlu_dist")
        superlu_dist_found = true;
      if (petsc_options_inames[i] == "-mat_superlu_dist_fact")
        fact_pattern_found = true;
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
  // SamePattern_SameRowPerm
  if (superlu_dist_found && !fact_pattern_found)
  {
    po.inames.push_back("-mat_superlu_dist_fact");
    po.values.push_back("SamePattern_SameRowPerm");
    pc_description += "mat_superlu_dist_fact: SamePattern_SameRowPerm";
  }
#endif
  // Set Preconditioner description
  po.pc_description = pc_description;
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

// Line Search Options
#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3, 3, 0)
  MooseEnum line_search("default cubic quadratic none basic basicnonorms", "default");
#else
  MooseEnum line_search("default shell none basic l2 bt cp", "default");
#endif
  std::string addtl_doc_str(" (Note: none = basic)");
#else
  MooseEnum line_search("default", "default");
  std::string addtl_doc_str("");
#endif
  params.addParam<MooseEnum>(
      "line_search", line_search, "Specifies the line search type" + addtl_doc_str);

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
      "-snes_test_display -snes_view -snew_ksp_ew",
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
                        "-snes_ls -snes_max_it -snes_rtol -snes_type -sub_ksp_type -sub_pc_type",
                        "",
                        true);
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
    mooseError("Error setting PETSc option.");
}

} // Namespace PetscSupport
} // Namespace MOOSE

#endif // LIBMESH_HAVE_PETSC
