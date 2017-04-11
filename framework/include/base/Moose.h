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

#ifndef MOOSE_H
#define MOOSE_H

// libMesh includes
#include "libmesh/perf_log.h"
#include "libmesh/libmesh_common.h"
#include "XTermConstants.h"

#include <string>
#include <utility>

using namespace libMesh;

class ActionFactory;
class Factory;

/**
 * MOOSE now contains C++11 code, so give a reasonable error message
 * stating the minimum required compiler versions.
 */
#ifndef LIBMESH_HAVE_CXX11
#error MOOSE requires a C++11 compatible compiler (GCC >= 4.8.4, Clang >= 3.4.0, Intel >= 20130607). Please update your compiler and try again.
#endif

/**
 * Testing a condition on a local CPU that need to be propagated across all processes.
 *
 * If the condition 'cond' is satisfied, it gets propagated across all processes, so the parallel
 * code take the same path (if that is requires).
 */
#define parallel_if                                                                                \
  (cond) bool __local_bool__ = (cond);                                                             \
  Parallel::max<bool>(__local_bool__);                                                             \
  if (__local_bool__)

/**
 * Wrap all fortran function calls in this.
 */
#ifdef __bg__ // On Blue Gene Architectures there is no underscore
#define FORTRAN_CALL(name) name
#else // One underscore everywhere else
#define FORTRAN_CALL(name) name##_
#endif

// forward declarations
class Syntax;
class FEProblemBase;

// Define MOOSE execution flags, this cannot be done in MooseTypes because the registration calls
// must be in Moose.C to remain consistent with other registration calls.
typedef int ExecFlagType;
extern const ExecFlagType EXEC_NONE;
extern const ExecFlagType EXEC_INITIAL;
extern const ExecFlagType EXEC_LINEAR;
extern const ExecFlagType EXEC_NONLINEAR;
extern const ExecFlagType EXEC_TIMESTEP_END;
extern const ExecFlagType EXEC_TIMESTEP_BEGIN;
extern const ExecFlagType EXEC_FINAL;
extern const ExecFlagType EXEC_FORCED;
extern const ExecFlagType EXEC_FAILED;
extern const ExecFlagType EXEC_CUSTOM;
extern const ExecFlagType EXEC_SUBDOMAIN;
extern const ExecFlagType EXEC_SAME_AS_MULTIAPP;

namespace Moose
{

/**
 * Perflog to be used by applications.
 * If the application prints this in the end they will get performance info.
 */
extern PerfLog perf_log;

/**
 * PerfLog to be used during setup.  This log will get printed just before the first solve. */
extern PerfLog setup_perf_log;

/**
 * Variable indicating whether we will enable FPE trapping for this run.
 */
extern bool _trap_fpe;

/**
 * Variable to toggle any warning into an error (includes deprecated code warnings)
 */
extern bool _warnings_are_errors;

/**
 * Variable to toggle only deprecated warnings as errors.
 */
extern bool _deprecated_is_error;

/**
 * Variable to turn on exceptions during mooseError() and mooseWarning(), should
 * only be used with MOOSE unit.
 */
extern bool _throw_on_error;

/**
 * Storage for execute flags.
 */
extern std::map<ExecFlagType, std::string> execute_flags;

/**
 * Macros for coloring any output stream (_console, std::ostringstream, etc.)
 */
#define COLOR_BLACK (Moose::colorConsole() ? XTERM_BLACK : "")
#define COLOR_RED (Moose::colorConsole() ? XTERM_RED : "")
#define COLOR_GREEN (Moose::colorConsole() ? XTERM_GREEN : "")
#define COLOR_YELLOW (Moose::colorConsole() ? XTERM_YELLOW : "")
#define COLOR_BLUE (Moose::colorConsole() ? XTERM_BLUE : "")
#define COLOR_MAGENTA (Moose::colorConsole() ? XTERM_MAGENTA : "")
#define COLOR_CYAN (Moose::colorConsole() ? XTERM_CYAN : "")
#define COLOR_WHITE (Moose::colorConsole() ? XTERM_WHITE : "")
#define COLOR_DEFAULT (Moose::colorConsole() ? XTERM_DEFAULT : "")

/// Returns whether Console coloring is turned on (default: true).
bool colorConsole();

/// Turns color escape sequences on/off for info written to stdout.
/// Returns the the set value which may be different than use_color.
bool setColorConsole(bool use_color, bool force = false);

/**
 * Import libMesh::out, and libMesh::err for use in MOOSE.
 */
using libMesh::out;
using libMesh::err;

/**
 * Register objects that are in MOOSE
 */
void registerObjects(Factory & factory);
void addActionTypes(Syntax & syntax);
void registerActions(Syntax & syntax, ActionFactory & action_factory);
void registerExecFlags();

void setSolverDefaults(FEProblemBase & problem);

/**
 * Swap the libMesh MPI communicator out for ours.
 */
MPI_Comm swapLibMeshComm(MPI_Comm new_comm);

void enableFPE(bool on = true);

// MOOSE Requires PETSc to run, this CPP check will cause a compile error if PETSc is not found
#ifndef LIBMESH_HAVE_PETSC
#error PETSc has not been detected, please ensure your environment is set up properly then rerun the libmesh build script and try to compile MOOSE again.
#endif

} // namespace Moose

/**
 * Function to mimic object registration macros.
 */
void registerExecFlag(const ExecFlagType & flag, const std::string & str);

#endif /* MOOSE_H */
