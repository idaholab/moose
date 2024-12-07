//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libMeshReducedNamespace.h"
#include "libmesh/perf_log.h"
#include "libmesh/libmesh_common.h"
#include "XTermConstants.h"

#include <memory>
#include <set>
#include <string>

namespace libMesh
{
template <typename>
class NumericVector;
template <typename>
class SparseMatrix;

// This was deprecated in libMesh a year ago!  It was obsolete 5 years
// ago!  How are 6 apps in CI still using it!?
#ifdef LIBMESH_ENABLE_DEPRECATED
template <typename T>
using UniquePtr = std::unique_ptr<T>;
#endif
}

class ActionFactory;
class Factory;
class MooseEnumItem;
class ExecFlagEnum;
class MooseVariableFieldBase;

void MooseVecView(libMesh::NumericVector<libMesh::Number> & vector);
void MooseVecView(const libMesh::NumericVector<libMesh::Number> & vector);
void MooseMatView(libMesh::SparseMatrix<libMesh::Number> & mat);
void MooseMatView(const libMesh::SparseMatrix<libMesh::Number> & mat);

/**
 * MOOSE now contains C++17 code, so give a reasonable error message
 * stating what the user can do to address this in their environment if C++17
 * compatibility isn't found.
 */
namespace Moose
{
static_assert(__cplusplus >= 201703L,
              "MOOSE requires a C++17 compatible compiler (GCC >= 7.5.0, Clang >= 5.0.2). Please "
              "update your compiler or, if compatible, add '-std=c++17' to your compiler flags "
              "and try again. If using the MOOSE conda package, please attempt a MOOSE environment "
              "update (using `mamba update moose-dev`). If this update is not successful, please "
              "create a new MOOSE environment (see "
              "https://mooseframework.inl.gov/getting_started/installation/"
              "conda.html#uninstall-conda-moose-environment).");
}

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
extern const ExecFlagType EXEC_NONLINEAR_CONVERGENCE;
extern const ExecFlagType EXEC_NONLINEAR;
extern const ExecFlagType EXEC_POSTCHECK;
extern const ExecFlagType EXEC_TIMESTEP_END;
extern const ExecFlagType EXEC_TIMESTEP_BEGIN;
extern const ExecFlagType EXEC_MULTIAPP_FIXED_POINT_BEGIN;
extern const ExecFlagType EXEC_MULTIAPP_FIXED_POINT_END;
extern const ExecFlagType EXEC_FINAL;
extern const ExecFlagType EXEC_FORCED;
extern const ExecFlagType EXEC_FAILED;
extern const ExecFlagType EXEC_CUSTOM;
extern const ExecFlagType EXEC_SUBDOMAIN;
extern const ExecFlagType EXEC_PRE_DISPLACE;
extern const ExecFlagType EXEC_SAME_AS_MULTIAPP;
extern const ExecFlagType EXEC_PRE_MULTIAPP_SETUP;
extern const ExecFlagType EXEC_TRANSFER;
extern const ExecFlagType EXEC_PRE_KERNELS;
extern const ExecFlagType EXEC_ALWAYS;

namespace Moose
{
// MOOSE is not tested with LIBMESH_DIM != 3
static_assert(LIBMESH_DIM == 3,
              "MOOSE must be built with a libmesh library compiled without --enable-1D-only "
              "or --enable-2D-only");

/**
 * This is the dimension of all vector and tensor datastructures used in MOOSE.
 * We enforce LIBMESH_DIM == 3 through a static assertion above.
 * Note that lower dimensional simulations embedded in 3D space can always be requested at runtime.
 */
static constexpr std::size_t dim = LIBMESH_DIM;

/**
 * Used by the signal handler to determine if we should write a checkpoint file out at any point
 * during operation.
 */
extern int interrupt_signal_number;

/**
 * Set to true (the default) to print the stack trace with error and warning
 * messages - false to omit it.
 */
extern bool show_trace;

/**
 * Set to false (the default) to display an error message only once for each error call code
 * location (as opposed to every time the code is executed).
 */
extern bool show_multiple;

/**
 * Perflog to be used by applications.
 * If the application prints this in the end they will get performance info.
 *
 * This is no longer instantiated in the framework and will be removed in the future.
 */
extern libMesh::PerfLog perf_log;

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
 * Variable to turn on exceptions during mooseError(), should only be used within MOOSE unit tests
 * or when about to perform threaded operations because exception throwing in threaded regions is
 * safe while aborting is inherently not when singletons are involved (e.g. what thread is
 * responsible for destruction, or what do you do about mutexes?)
 */
extern bool _throw_on_error;

/**
 * Variable to turn on exceptions during mooseWarning(), should
 * only be used in MOOSE unit tests.
 */
extern bool _throw_on_warning;

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
