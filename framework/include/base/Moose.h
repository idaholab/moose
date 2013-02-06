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
#include "perf_log.h"
#include "parallel.h"

#include <string>

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

// forward declarations
class Syntax;
class FEProblem;

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
 * Register objects that are in MOOSE
 */
void registerObjects(Factory & factory);
void addActionTypes(Syntax & syntax);
void registerActions(Syntax & syntax, ActionFactory & action_factory);

void setSolverDefaults(FEProblem & problem);

} // namespace Moose


#define LENGTHOF(a) (sizeof(a)/sizeof(a[0]))

#endif /* MOOSE_H */
