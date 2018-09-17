//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSE_H
#define MOOSE_H

#include "libmesh/perf_log.h"
#include "libmesh/libmesh_common.h"
#include "XTermConstants.h"

#include <set>
#include <string>

using namespace libMesh;

class ActionFactory;
class Factory;
class MooseEnumItem;
class ExecFlagEnum;
class MooseVariableFEBase;

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

/**
 * Function to mirror the behavior of the C++17 std::map::try_emplace() method (no hint).
 * @param m The std::map
 * @param k The key use to insert the pair
 * @param args The value to be inserted. This can be a moveable type but won't be moved
 *             if the insertion is successful.
 */
template <class M, class... Args>
std::pair<typename M::iterator, bool>
moose_try_emplace(M & m, const typename M::key_type & k, Args &&... args)
{
  auto it = m.lower_bound(k);
  if (it == m.end() || m.key_comp()(k, it->first))
  {
    return {m.emplace_hint(it,
                           std::piecewise_construct,
                           std::forward_as_tuple(k),
                           std::forward_as_tuple(std::forward<Args>(args)...)),
            true};
  }
  return {it, false};
}

// forward declarations
class Syntax;
class FEProblemBase;

// Define MOOSE execution flags, this cannot be done in MooseTypes because the registration calls
// must be in Moose.C to remain consistent with other registration calls.
using ExecFlagType = MooseEnumItem;
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
 * Storage for the registered execute flags. This is needed for the ExecuteMooseObjectWarehouse
 * to create the necessary storage containers on a per flag basis. This isn't something that
 * should be used by application developers.
 */
extern ExecFlagEnum execute_flags;

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
using libMesh::err;
using libMesh::out;

/**
 * Register objects that are in MOOSE
 */

void registerAll(Factory & f, ActionFactory & af, Syntax & s);

void registerObjects(Factory & factory);
void registerObjects(Factory & factory, const std::set<std::string> & obj_labels);
void addActionTypes(Syntax & syntax);
void registerActions(Syntax & syntax, ActionFactory & action_factory);
void registerActions(Syntax & syntax,
                     ActionFactory & action_factory,
                     const std::set<std::string> & obj_labels);
void registerExecFlags(Factory & factory);

void associateSyntax(Syntax & syntax, ActionFactory & action_factory);

void setSolverDefaults(FEProblemBase & problem);

/**
 * Swap the libMesh MPI communicator out for ours.  Note that you should usually use
 * the Moose::ScopedCommSwapper class instead of calling this function.
 */
MPI_Comm swapLibMeshComm(MPI_Comm new_comm);

class ScopedCommSwapper
{
public:
  /// Swaps the current libmesh MPI communicator for new_comm.  new_comm will be automatically
  /// swapped back in as the current libmesh communicator when this object is destructed.
  ScopedCommSwapper(MPI_Comm new_comm) : _orig(swapLibMeshComm(new_comm)) {}
  virtual ~ScopedCommSwapper() { swapLibMeshComm(_orig); }
  /// Forcibly swap the currently swapped-out communicator back in to libmesh.  Calling this
  /// function twice in a row leaves communicators exactly as they were before this function
  /// was called.  Usually you should not need/use this function because MPI communicators
  /// are swapped automatically when this object is constructed/destructed.
  void forceSwap() { _orig = swapLibMeshComm(_orig); }

private:
  MPI_Comm _orig;
};

// MOOSE Requires PETSc to run, this CPP check will cause a compile error if PETSc is not found
#ifndef LIBMESH_HAVE_PETSC
#error PETSc has not been detected, please ensure your environment is set up properly then rerun the libmesh build script and try to compile MOOSE again.
#endif

} // namespace Moose

#endif /* MOOSE_H */
