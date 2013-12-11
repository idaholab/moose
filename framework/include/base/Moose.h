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
#include "libmesh/parallel.h"
#include "libmesh/libmesh_common.h"

#include <string>

using namespace libMesh;

class ActionFactory;
class Factory;

/**
 * Testing a condition on a local CPU that need to be propagated across all processes.
 *
 * If the condition 'cond' is satisfied, it gets propagated across all processes, so the parallel code take the same path (if that is requires).
 */
#define parallel_if(cond)                       \
    bool __local_bool__ = (cond);               \
    Parallel::max<bool>(__local_bool__);        \
    if (__local_bool__)

/**
 * Wrap all fortran function calls in this.
 */
#ifdef __bg__ // On Blue Gene Architectures there is no underscore
  #define FORTRAN_CALL(name) name
#else  // One underscore everywhere else
  #define FORTRAN_CALL(name) name ## _
#endif

// forward declarations
class Syntax;
class FEProblem;

/// Execution flags - when is the object executed/evaluated
enum ExecFlagType {
  /// Object is evaluated only once at the beginning of the simulation
  EXEC_INITIAL,
  /// Object is evaluated in every residual computation
  EXEC_RESIDUAL,
  /// Object is evaluated in every jacobian computation
  EXEC_JACOBIAN,
  /// Object is evaluated at the end of every time step
  EXEC_TIMESTEP,
  /// Object is evaluated at the beginning of every time step
  EXEC_TIMESTEP_BEGIN,
  /// For use with custom executioners that want to fire objects at a specific time
  EXEC_CUSTOM
};

namespace Moose
{

/**
 * Perflog to be used by applications.
 * If the application prints this in the end they will get performance info.
 */
extern PerfLog perf_log;

/**
 * PerfLog to be used during setup.  This log will get printed just before the first solve.
 */
extern PerfLog setup_perf_log;

/**
 * Variable indicating whether we will enable FPE trapping for this run.
 */
extern bool __trap_fpe;

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

void setSolverDefaults(FEProblem & problem);

/**
 * Swap the libMesh MPI communicator out for ours.
 */
MPI_Comm swapLibMeshComm(MPI_Comm new_comm);

void enableFPE(bool on = true);

} // namespace Moose


#define LENGTHOF(a) (sizeof(a)/sizeof(a[0]))

#endif /* MOOSE_H */
