//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "ParallelParamObject.h"

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
  PetscMatrixBase<Number> & petsc_mat = static_cast<PetscMatrix<Number> &>(mat);
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
  PetscMatrixBase<Number> & petsc_mat =
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
setSolverOptions(const SolverParams & solver_params, const MultiMooseEnum & dont_add_these_options)
{
  // set PETSc options implied by a solve type
  switch (solver_params._type)
  {
    case Moose::ST_PJFNK:
      setSinglePetscOptionIfAppropriate(dont_add_these_options,
                                        solver_params._prefix + "snes_mf_operator");
      setSinglePetscOptionIfAppropriate(dont_add_these_options,
                                        solver_params._prefix + "mat_mffd_type",
                                        stringify(solver_params._mffd_type));
      break;

    case Moose::ST_JFNK:
      setSinglePetscOptionIfAppropriate(dont_add_these_options, solver_params._prefix + "snes_mf");
      setSinglePetscOptionIfAppropriate(dont_add_these_options,
                                        solver_params._prefix + "mat_mffd_type",
                                        stringify(solver_params._mffd_type));
      break;

    case Moose::ST_NEWTON:
      break;

    case Moose::ST_FD:
      setSinglePetscOptionIfAppropriate(dont_add_these_options, solver_params._prefix + "snes_fd");
      break;

    case Moose::ST_LINEAR:
      setSinglePetscOptionIfAppropriate(
          dont_add_these_options, solver_params._prefix + "snes_type", "ksponly");
      setSinglePetscOptionIfAppropriate(dont_add_these_options,
                                        solver_params._prefix + "snes_monitor_cancel");
      break;
  }

  Moose::LineSearchType ls_type = solver_params._line_search;
  if (ls_type == Moose::LS_NONE)
    ls_type = Moose::LS_BASIC;

  if (ls_type != Moose::LS_DEFAULT && ls_type != Moose::LS_CONTACT && ls_type != Moose::LS_PROJECT)
    setSinglePetscOptionIfAppropriate(
        dont_add_these_options, solver_params._prefix + "snes_linesearch_type", stringify(ls_type));
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
petscSetOptionsHelper(const PetscOptions & po, FEProblemBase * const problem)
{
  // Add any additional options specified in the input file
  for (const auto & flag : po.flags)
    // Need to use name method here to pass a str instead of an EnumItem because
    // we don't care if the id attributes match
    if (!po.dont_add_these_options.contains(flag.name()) ||
        po.user_set_options.contains(flag.name()))
      setSinglePetscOption(flag.rawName().c_str());

  // Add option pairs
  for (auto & option : po.pairs)
    if (!po.dont_add_these_options.contains(option.first) ||
        po.user_set_options.contains(option.first))
      setSinglePetscOption(option.first, option.second, problem);

  addPetscOptionsFromCommandline();
}

void
petscSetOptions(const PetscOptions & po,
                const SolverParams & solver_params,
                FEProblemBase * const problem)
{
  PetscCallAbort(PETSC_COMM_WORLD, PetscOptionsClear(LIBMESH_PETSC_NULLPTR));
  setSolverOptions(solver_params, po.dont_add_these_options);
  petscSetOptionsHelper(po, problem);
}

void
petscSetOptions(const PetscOptions & po,
                const std::vector<SolverParams> & solver_params_vec,
                FEProblemBase * const problem)
{
  PetscCallAbort(PETSC_COMM_WORLD, PetscOptionsClear(LIBMESH_PETSC_NULLPTR));
  for (const auto & solver_params : solver_params_vec)
    setSolverOptions(solver_params, po.dont_add_these_options);
  petscSetOptionsHelper(po, problem);
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

PetscErrorCode
petscLinearConverged(
    KSP /*ksp*/, PetscInt it, PetscReal /*norm*/, KSPConvergedReason * reason, void * ctx)
{
  PetscFunctionBegin;
  FEProblemBase & problem = *static_cast<FEProblemBase *>(ctx);

  // execute objects that may be used in convergence check
  // Right now, setting objects to execute on this flag would be ignored except in the
  // linear-system-only use case.
  problem.execute(EXEC_LINEAR_CONVERGENCE);

  // perform the convergence check
  Convergence::MooseConvergenceStatus status;
  if (problem.getFailNextSystemConvergenceCheck())
  {
    status = Convergence::MooseConvergenceStatus::DIVERGED;
    problem.resetFailNextSystemConvergenceCheck();
  }
  else
  {
    auto & convergence = problem.getConvergence(
        problem.getLinearConvergenceNames()[problem.currentLinearSystem().number()]);
    status = convergence.checkConvergence(it);
  }

  // convert convergence status to PETSc converged reason
  switch (status)
  {
    case Convergence::MooseConvergenceStatus::ITERATING:
      *reason = KSP_CONVERGED_ITERATING;
      break;

      // TODO: find a KSP code that works better for this case
    case Convergence::MooseConvergenceStatus::CONVERGED:
      *reason = KSP_CONVERGED_RTOL_NORMAL;
      break;

    case Convergence::MooseConvergenceStatus::DIVERGED:
      *reason = KSP_DIVERGED_DTOL;
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
  // We care about both nonlinear and linear systems when setting the SNES prefix because
  // SNESSetOptionsPrefix will also set its KSP prefix which could compete with linear system KSPs
  for (const auto nl_index : make_range(problem.numNonlinearSystems()))
  {
    NonlinearSystemBase & nl = problem.getNonlinearSystemBase(nl_index);

    //
    // prefix system matrices
    //

    auto & lm_sys = nl.system();
    // This check is necessary because we still have at least the system matrix lying around even
    // when doing matrix-free, but critically even though the libmesh object(s) exist, the wrapped
    // PETSc Mat(s) do not
    if (problem.solverParams(nl_index)._type != Moose::ST_JFNK)
      for (auto & [mat_name, mat] : as_range(lm_sys.matrices_begin(), lm_sys.matrices_end()))
      {
        libmesh_ignore(mat_name);
        if (auto * const petsc_mat = dynamic_cast<PetscMatrixBase<Number> *>(mat.get()); petsc_mat)
        {
          LibmeshPetscCallA(nl.comm().get(),
                            MatSetOptionsPrefix(petsc_mat->mat(), (nl.name() + "_").c_str()));
          // We should call this here to ensure that options from the command line are properly
          // applied
          LibmeshPetscCallA(nl.comm().get(), MatSetFromOptions(petsc_mat->mat()));
        }
      }

    //
    // prefix SNES/KSP
    //

    // dig out PETSc solver
    auto * const petsc_solver = cast_ptr<PetscNonlinearSolver<Number> *>(nl.nonlinearSolver());
    const char * snes_prefix = nullptr;
    std::string snes_prefix_str;
    if (nl.system().prefix_with_name())
    {
      snes_prefix_str = nl.system().prefix();
      snes_prefix = snes_prefix_str.c_str();
    }
    SNES snes = petsc_solver->snes(snes_prefix);
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

    if (problem.hasLinearConvergenceObjects())
      LibmeshPetscCallA(
          lin_sys.comm().get(),
          KSPSetConvergenceTest(ksp, petscLinearConverged, &problem, LIBMESH_PETSC_NULLPTR));

    // We dont set the KSP defaults here because they seem to clash with the linear solve parameters
    // set in FEProblemBase::solveLinearSystem
  }
}

void
processSingletonMooseWrappedOptions(FEProblemBase & fe_problem, const InputParameters & params)
{
  setSolveTypeFromParams(fe_problem, params);
  setLineSearchFromParams(fe_problem, params);
  setMFFDTypeFromParams(fe_problem, params);
}

#define checkPrefix(prefix)                                                                        \
  mooseAssert(prefix[0] == '-',                                                                    \
              "Leading prefix character must be a '-'. Current prefix is '" << prefix << "'");     \
  mooseAssert((prefix.size() == 1) || (prefix.back() == '_'),                                      \
              "Terminating prefix character must be a '_'. Current prefix is '" << prefix << "'"); \
  mooseAssert(MooseUtils::isAllLowercase(prefix), "PETSc prefixes should be all lower-case")

void
storePetscOptions(FEProblemBase & fe_problem,
                  const std::string & prefix,
                  const ParallelParamObject & param_object)
{
  const auto & params = param_object.parameters();
  processSingletonMooseWrappedOptions(fe_problem, params);

  // The parameters contained in the Action
  const auto & petsc_options = params.get<MultiMooseEnum>("petsc_options");
  const auto & petsc_pair_options =
      params.get<MooseEnumItem, std::string>("petsc_options_iname", "petsc_options_value");

  // A reference to the PetscOptions object that contains the settings that will be used in the
  // solve
  auto & po = fe_problem.getPetscOptions();

  // First process the single petsc options/flags
  addPetscFlagsToPetscOptions(petsc_options, prefix, param_object, po);

  // Then process the option-value pairs
  addPetscPairsToPetscOptions(
      petsc_pair_options, fe_problem.mesh().dimension(), prefix, param_object, po);
}

void
setSolveTypeFromParams(FEProblemBase & fe_problem, const InputParameters & params)
{
  // Note: Options set in the Preconditioner block will override those set in the Executioner block
  if (params.isParamValid("solve_type") && !params.isParamValid("_use_eigen_value"))
  {
    // Extract the solve type
    const std::string & solve_type = params.get<MooseEnum>("solve_type");
    for (const auto i : make_range(fe_problem.numNonlinearSystems()))
      fe_problem.solverParams(i)._type = Moose::stringToEnum<Moose::SolveType>(solve_type);
  }
}

void
setLineSearchFromParams(FEProblemBase & fe_problem, const InputParameters & params)
{
  // Note: Options set in the Preconditioner block will override those set in the Executioner block
  if (params.isParamValid("line_search"))
  {
    const auto & line_search = params.get<MooseEnum>("line_search");
    for (const auto i : make_range(fe_problem.numNonlinearSystems()))
      if (fe_problem.solverParams(i)._line_search == Moose::LS_INVALID || line_search != "default")
      {
        Moose::LineSearchType enum_line_search =
            Moose::stringToEnum<Moose::LineSearchType>(line_search);
        fe_problem.solverParams(i)._line_search = enum_line_search;
        if (enum_line_search == LS_CONTACT || enum_line_search == LS_PROJECT)
        {
          NonlinearImplicitSystem * nl_system = dynamic_cast<NonlinearImplicitSystem *>(
              &fe_problem.getNonlinearSystemBase(i).system());
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
}

void
setMFFDTypeFromParams(FEProblemBase & fe_problem, const InputParameters & params)
{
  if (params.isParamValid("mffd_type"))
  {
    const auto & mffd_type = params.get<MooseEnum>("mffd_type");
    for (const auto i : make_range(fe_problem.numNonlinearSystems()))
      fe_problem.solverParams(i)._mffd_type = Moose::stringToEnum<Moose::MffdType>(mffd_type);
  }
}

template <typename T>
void
checkUserProvidedPetscOption(const T & option, const ParallelParamObject & param_object)
{
  const auto & string_option = static_cast<const std::string &>(option);
  if (string_option[0] != '-')
    param_object.mooseError("PETSc option '", string_option, "' does not begin with '-'");
}

void
addPetscFlagsToPetscOptions(const MultiMooseEnum & petsc_flags,
                            const std::string & prefix,
                            const ParallelParamObject & param_object,
                            PetscOptions & po)
{
  checkPrefix(prefix);

  // Update the PETSc single flags
  for (const auto & option : petsc_flags)
  {
    checkUserProvidedPetscOption(option, param_object);

    const std::string & string_option = option.name();

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

    // Update the stored items, but do not create duplicates
    const std::string prefixed_option = prefix + string_option.substr(1);
    if (!po.flags.isValueSet(prefixed_option))
    {
      po.flags.setAdditionalValue(prefixed_option);
      po.user_set_options.setAdditionalValue(prefixed_option);
    }
  }
}

void
setConvergedReasonFlags(FEProblemBase & fe_problem, const std::string & prefix)
{
  checkPrefix(prefix);
  libmesh_ignore(fe_problem); // avoid unused warnings for old PETSc

#if !PETSC_VERSION_LESS_THAN(3, 14, 0)
  // the boolean in these pairs denote whether the user has specified any of the reason flags in the
  // input file
  std::array<std::string, 2> reason_flags = {{"snes_converged_reason", "ksp_converged_reason"}};

  auto & po = fe_problem.getPetscOptions();

  for (const auto & reason_flag : reason_flags)
    if (!po.flags.isValueSet(prefix + reason_flag) &&
        (std::find_if(po.pairs.begin(),
                      po.pairs.end(),
                      [&reason_flag, &prefix](auto & pair)
                      { return pair.first == (prefix + reason_flag); }) == po.pairs.end()))
      po.pairs.emplace_back(prefix + reason_flag, "::failed");
#endif
}

void
addPetscPairsToPetscOptions(
    const std::vector<std::pair<MooseEnumItem, std::string>> & petsc_pair_options,
    const unsigned int mesh_dimension,
    const std::string & prefix,
    const ParallelParamObject & param_object,
    PetscOptions & po)
{
  checkPrefix(prefix);

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

  for (const auto & [option_name, option_value] : petsc_pair_options)
  {
    checkUserProvidedPetscOption(option_name, param_object);

    new_options.clear();
    const std::string prefixed_option_name =
        prefix + static_cast<const std::string &>(option_name).substr(1);

    // Do not add duplicate settings
    if (auto it =
            MooseUtils::findPair(po.pairs, po.pairs.begin(), prefixed_option_name, MooseUtils::Any);
        it == po.pairs.end())
    {
#if !PETSC_VERSION_LESS_THAN(3, 9, 0)
      if (option_name == "-pc_factor_mat_solver_package")
        new_options.emplace_back(prefix + "pc_factor_mat_solver_type", option_value);
#else
      if (option_name == "-pc_factor_mat_solver_type")
        new_options.push_back(prefix + "pc_factor_mat_solver_package", option_value);
#endif

      // Look for a pc description
      if (option_name == "-pc_type" || option_name == "-sub_pc_type" ||
          option_name == "-pc_hypre_type")
        pc_description += option_value + ' ';

#if !PETSC_VERSION_LESS_THAN(3, 12, 0)
      if (option_name == "-pc_type" && option_value == "hmg")
        hmg_found = true;

        // MPIAIJ for PETSc 3.12.0: -matptap_via
        // MAIJ for PETSc 3.12.0: -matmaijptap_via
        // MPIAIJ for PETSc 3.13 to 3.16: -matptap_via, -matproduct_ptap_via
        // MAIJ for PETSc 3.13 to 3.16: -matproduct_ptap_via
        // MPIAIJ for PETSc 3.17 and higher: -matptap_via, -mat_product_algorithm
        // MAIJ for PETSc 3.17 and higher: -mat_product_algorithm
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
      if (hmg_found && (option_name == "-matptap_via" || option_name == "-matmaijptap_via" ||
                        option_name == "-matproduct_ptap_via"))
        new_options.emplace_back(prefix + "mat_product_algorithm", option_value);
#elif !PETSC_VERSION_LESS_THAN(3, 13, 0)
      if (hmg_found && (option_name == "-matptap_via" || option_name == "-matmaijptap_via"))
        new_options.emplace_back(prefix + "matproduct_ptap_via", option_value);
#else
      if (hmg_found && (option_name == "-matproduct_ptap_via"))
      {
        new_options.emplace_back(prefix + "matptap_via", option_value);
        new_options.emplace_back(prefix + "matmaijptap_via", option_value);
      }
#endif

      if (option_name == "-matptap_via" || option_name == "-matmaijptap_via" ||
          option_name == "-matproduct_ptap_via" || option_name == "-mat_product_algorithm")
        matptap_found = true;

      // For 3D problems, we need to set this 0.7
      if (option_name == "-hmg_inner_pc_hypre_boomeramg_strong_threshold")
        hmg_strong_threshold_found = true;
#endif
      // This special case is common enough that we'd like to handle it for the user.
      if (option_name == "-pc_hypre_type" && option_value == "boomeramg")
        boomeramg_found = true;
      if (option_name == "-pc_hypre_boomeramg_strong_threshold")
        strong_threshold_found = true;
#if !PETSC_VERSION_LESS_THAN(3, 7, 0)
      if ((option_name == "-pc_factor_mat_solver_package" ||
           option_name == "-pc_factor_mat_solver_type") &&
          option_value == "superlu_dist")
        superlu_dist_found = true;
      if (option_name == "-mat_superlu_dist_fact")
        fact_pattern_found = true;
      if (option_name == "-mat_superlu_dist_replacetinypivot")
        tiny_pivot_found = true;
#endif

      if (!new_options.empty())
      {
        std::copy(new_options.begin(), new_options.end(), std::back_inserter(po.pairs));
        for (const auto & option : new_options)
          po.user_set_options.setAdditionalValue(option.first);
      }
      else
      {
        po.pairs.push_back(std::make_pair(prefixed_option_name, option_value));
        po.user_set_options.setAdditionalValue(prefixed_option_name);
      }
    }
    else
    {
      do
      {
        it->second = option_value;
        it = MooseUtils::findPair(po.pairs, std::next(it), prefixed_option_name, MooseUtils::Any);
      } while (it != po.pairs.end());
    }
  }

  // When running a 3D mesh with boomeramg, it is almost always best to supply a strong threshold
  // value. We will provide that for the user here if they haven't supplied it themselves.
  if (boomeramg_found && !strong_threshold_found && mesh_dimension == 3)
  {
    po.pairs.emplace_back(prefix + "pc_hypre_boomeramg_strong_threshold", "0.7");
    pc_description += "strong_threshold: 0.7 (auto)";
  }

#if !PETSC_VERSION_LESS_THAN(3, 12, 0)
  if (hmg_found && !hmg_strong_threshold_found && mesh_dimension == 3)
  {
    po.pairs.emplace_back(prefix + "hmg_inner_pc_hypre_boomeramg_strong_threshold", "0.7");
    pc_description += "strong_threshold: 0.7 (auto)";
  }

  // Default PETSc PtAP takes too much memory, and it is not quite useful
  // Let us switch to use new algorithm
  if (hmg_found && !matptap_found)
  {
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
    po.pairs.emplace_back(prefix + "mat_product_algorithm", "allatonce");
#elif !PETSC_VERSION_LESS_THAN(3, 13, 0)
    po.pairs.emplace_back(prefix + "matproduct_ptap_via", "allatonce");
#else
    po.pairs.emplace_back(prefix + "matptap_via", "allatonce");
    po.pairs.emplace_back(prefix + "matmaijptap_via", "allatonce");
#endif
  }
#endif

#if !PETSC_VERSION_LESS_THAN(3, 7, 0)
  // In PETSc-3.7.{0--4}, there is a bug when using superlu_dist, and we have to use
  // SamePattern_SameRowPerm, otherwise we use whatever we have in PETSc
  if (superlu_dist_found && !fact_pattern_found)
  {
    po.pairs.emplace_back(prefix + "mat_superlu_dist_fact",
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
    po.pairs.emplace_back(prefix + "mat_superlu_dist_replacetinypivot", "1");
    pc_description += " mat_superlu_dist_replacetinypivot: true ";
  }
#endif
  // Set Preconditioner description
  if (!pc_description.empty() && prefix.size() > 1)
    po.pc_description += "[" + prefix.substr(1, prefix.size() - 2) + "]: ";
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
getCommonSNESFlags()
{
  return MultiMooseEnum(
      "-ksp_monitor_snes_lg -snes_ksp_ew -ksp_snes_ew -snes_converged_reason "
      "-snes_ksp -snes_linesearch_monitor -snes_mf -snes_mf_operator -snes_monitor "
      "-snes_test_display -snes_view -snes_monitor_cancel",
      "",
      true);
}

MultiMooseEnum
getCommonKSPFlags()
{
  return MultiMooseEnum(
      "-ksp_converged_reason -ksp_gmres_modifiedgramschmidt -ksp_monitor", "", true);
}

MultiMooseEnum
getCommonPetscFlags()
{
  auto options = MultiMooseEnum("-dm_moose_print_embedding -dm_view", "", true);
  options.addValidName(getCommonKSPFlags());
  options.addValidName(getCommonSNESFlags());
  return options;
}

MultiMooseEnum
getCommonSNESKeys()
{
  return MultiMooseEnum("-snes_atol -snes_linesearch_type -snes_ls -snes_max_it -snes_rtol "
                        "-snes_divergence_tolerance -snes_type",
                        "",
                        true);
}

MultiMooseEnum
getCommonKSPKeys()
{
  return MultiMooseEnum("-ksp_atol -ksp_gmres_restart -ksp_max_it -ksp_pc_side -ksp_rtol "
                        "-ksp_type -sub_ksp_type",
                        "",
                        true);
}
MultiMooseEnum
getCommonPetscKeys()
{
  auto options = MultiMooseEnum("-mat_fd_coloring_err -mat_fd_type -mat_mffd_type "
                                "-pc_asm_overlap -pc_factor_levels "
                                "-pc_factor_mat_ordering_type -pc_hypre_boomeramg_grid_sweeps_all "
                                "-pc_hypre_boomeramg_max_iter "
                                "-pc_hypre_boomeramg_strong_threshold -pc_hypre_type -pc_type "
                                "-sub_pc_type",
                                "",
                                true);
  options.addValidName(getCommonKSPKeys());
  options.addValidName(getCommonSNESKeys());
  return options;
}

bool
isSNESVI(FEProblemBase & fe_problem)
{
  const PetscOptions & petsc = fe_problem.getPetscOptions();

  int argc;
  char ** args;
  LibmeshPetscCallA(fe_problem.comm().get(), PetscGetArgs(&argc, &args));

  std::vector<std::string> cml_arg;
  for (int i = 0; i < argc; i++)
    cml_arg.push_back(args[i]);

  if (MooseUtils::findPair(petsc.pairs, petsc.pairs.begin(), MooseUtils::Any, "vinewtonssls") ==
          petsc.pairs.end() &&
      MooseUtils::findPair(petsc.pairs, petsc.pairs.begin(), MooseUtils::Any, "vinewtonrsls") ==
          petsc.pairs.end() &&
      std::find(cml_arg.begin(), cml_arg.end(), "vinewtonssls") == cml_arg.end() &&
      std::find(cml_arg.begin(), cml_arg.end(), "vinewtonrsls") == cml_arg.end())
    return false;

  return true;
}

void
setSinglePetscOption(const std::string & name,
                     const std::string & value /*=""*/,
                     FEProblemBase * const problem /*=nullptr*/)
{
  static const TIMPI::Communicator comm_world(PETSC_COMM_WORLD);
  const TIMPI::Communicator & comm = problem ? problem->comm() : comm_world;
  LibmeshPetscCallA(comm.get(),
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
        LibmeshPetscCallA(comm.get(), VecSetFromOptions(petsc_vec->vec()));
      }
      // The solution vectors aren't included in the system vectors storage
      auto * petsc_vec = cast_ptr<PetscVector<Number> *>(lm_sys.solution.get());
      LibmeshPetscCallA(comm.get(), VecSetFromOptions(petsc_vec->vec()));
      petsc_vec = cast_ptr<PetscVector<Number> *>(lm_sys.current_local_solution.get());
      LibmeshPetscCallA(comm.get(), VecSetFromOptions(petsc_vec->vec()));
    }
  }
  // Select matrix type from user-passed PETSc options
  else if (lower_case_name.find("mat_type") != std::string::npos)
  {
    check_problem();

    bool found_matching_prefix = false;
    for (const auto i : index_range(problem->_solver_systems))
    {
      const auto & solver_sys_name = problem->_solver_sys_names[i];
      if (lower_case_name.find("-" + MooseUtils::toLower(solver_sys_name) + "_mat_type") ==
          std::string::npos)
        continue;

      if (problem->solverParams(i)._type == Moose::ST_JFNK)
        mooseError(
            "Setting option '", lower_case_name, "' is incompatible with a JFNK 'solve_type'");

      auto & lm_sys = problem->_solver_systems[i]->system();
      for (auto & [mat_name, mat] : as_range(lm_sys.matrices_begin(), lm_sys.matrices_end()))
      {
        libmesh_ignore(mat_name);
        if (auto * const petsc_mat = dynamic_cast<PetscMatrixBase<Number> *>(mat.get()); petsc_mat)
        {
#ifdef DEBUG
          const char * mat_prefix;
          LibmeshPetscCallA(comm.get(), MatGetOptionsPrefix(petsc_mat->mat(), &mat_prefix));
          mooseAssert(strcmp(mat_prefix, (solver_sys_name + "_").c_str()) == 0,
                      "We should have prefixed these matrices previously");
#endif
          LibmeshPetscCallA(comm.get(), MatSetFromOptions(petsc_mat->mat()));
        }
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
setSinglePetscOptionIfAppropriate(const MultiMooseEnum & dont_add_these_options,
                                  const std::string & name,
                                  const std::string & value /*=""*/,
                                  FEProblemBase * const problem /*=nullptr*/)
{
  if (!dont_add_these_options.contains(name))
    setSinglePetscOption(name, value, problem);
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
dontAddPetscFlag(const std::string & flag, PetscOptions & petsc_options)
{
  if (!petsc_options.dont_add_these_options.contains(flag))
    petsc_options.dont_add_these_options.setAdditionalValue(flag);
}

void
dontAddNonlinearConvergedReason(FEProblemBase & fe_problem)
{
  dontAddPetscFlag("-snes_converged_reason", fe_problem.getPetscOptions());
}

void
dontAddLinearConvergedReason(FEProblemBase & fe_problem)
{
  dontAddPetscFlag("-ksp_converged_reason", fe_problem.getPetscOptions());
}

void
dontAddCommonKSPOptions(FEProblemBase & fe_problem)
{
  auto & petsc_options = fe_problem.getPetscOptions();
  for (const auto & flag : getCommonKSPFlags().getNames())
    dontAddPetscFlag(flag, petsc_options);
  for (const auto & key : getCommonKSPKeys().getNames())
    dontAddPetscFlag(key, petsc_options);
}

void
dontAddCommonSNESOptions(FEProblemBase & fe_problem)
{
  auto & petsc_options = fe_problem.getPetscOptions();
  for (const auto & flag : getCommonSNESFlags().getNames())
    if (!petsc_options.dont_add_these_options.contains(flag))
      petsc_options.dont_add_these_options.setAdditionalValue(flag);
  for (const auto & key : getCommonSNESKeys().getNames())
    if (!petsc_options.dont_add_these_options.contains(key))
      petsc_options.dont_add_these_options.setAdditionalValue(key);
}

std::unique_ptr<PetscMatrix<Number>>
createMatrixFromFile(const libMesh::Parallel::Communicator & comm,
                     Mat & mat,
                     const std::string & binary_mat_file,
                     const unsigned int mat_number_to_load)
{
  LibmeshPetscCallA(comm.get(), MatCreate(comm.get(), &mat));
  PetscViewer matviewer;
  LibmeshPetscCallA(
      comm.get(),
      PetscViewerBinaryOpen(comm.get(), binary_mat_file.c_str(), FILE_MODE_READ, &matviewer));
  for (unsigned int i = 0; i < mat_number_to_load; ++i)
    LibmeshPetscCallA(comm.get(), MatLoad(mat, matviewer));
  LibmeshPetscCallA(comm.get(), PetscViewerDestroy(&matviewer));

  return std::make_unique<PetscMatrix<Number>>(mat, comm);
}

} // Namespace PetscSupport
} // Namespace MOOSE
