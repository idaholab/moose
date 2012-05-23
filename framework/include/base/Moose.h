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
#include "libmesh_config.h"
#include "print_trace.h"
#include "libmesh_common.h"
#include "perf_log.h"
#include "mtwist.h"
#include "id_types.h"
#include "stored_range.h"
#include "parallel.h"

#include <string>

/**
 * MOOSE typedefs
 */
typedef Real                     PostprocessorValue;
typedef boundary_id_type         BoundaryID;
typedef std::string              BoundaryName;
typedef subdomain_id_type        SubdomainID;
typedef std::string              SubdomainName;

/**
 * MOOSE wrapped versions of useful libMesh macros (see libmesh_common.h)
 */
#ifdef GRACEFUL_ERROR
#define mooseError(msg) do { std::cerr << "\n\n" << msg << "\n\n"; print_trace(); exit(1); } while(0)
#else
#define mooseError(msg) do { std::cerr << "\n\n" << msg << "\n\n"; print_trace(); libmesh_error(); } while(0)
#endif

#ifdef NDEBUG
#define mooseAssert(asserted, msg)
#else
#define mooseAssert(asserted, msg)  do { if (!(asserted)) { std::cerr << "\n\nAssertion `" #asserted "' failed\n" << msg << "\nat " << __FILE__ << ", line " << __LINE__ << std::endl; print_trace(); libmesh_error(); } } while(0)
#endif

#define mooseWarning(msg) do { std::cerr << "\n\n*** Warning ***\n" << msg << "\nat " << __FILE__ << ", line " << __LINE__ << "\n" << std::endl; } while(0)

#define mooseDoOnce(do_this) do { static bool did_this_already = false; if (!did_this_already) { did_this_already = true; do_this; } } while (0)

#define mooseDeprecated() mooseDoOnce(std::cout << "*** Warning, This code is deprecated, and likely to be removed in future library versions! " << __FILE__ << ", line " << __LINE__ << ", compiled " << __DATE__ << " at " << __TIME__ << " ***" << std::endl;)

/**
 * Testing a condition on a local CPU that need to be propagated across all processes.
 *
 * If the condition 'cond' is satisfied, it gets propagated across all processes, so the parallel code take the same path (if that is requires).
 */
#define parallel_if(cond)                       \
    bool __local_bool__ = (cond);               \
    Parallel::max<bool>(__local_bool__);        \
    if (__local_bool__)


class FEProblem;
class ActionWarehouse;
class Executioner;

typedef StoredRange<std::vector<unsigned int>::iterator, unsigned int> NodeIdRange;

namespace Moose
{

extern ActionWarehouse action_warehouse;

extern Executioner *executioner;

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
void registerObjects();
void addActionTypes();
void registerActions();

void setSolverDefaults(FEProblem & problem);

/**
 * Framework-wide stuff
 */

enum VarKindType
{
  VAR_NONLINEAR,
  VAR_AUXILIARY
};

enum TimeSteppingScheme
{
  IMPLICIT_EULER,
  BDF2,
  CRANK_NICOLSON
};

// Bit mask flags to be able to combine them through or-operator (|)
enum PostprocessorType
{
  PPS_RESIDUAL = 0x01,
  PPS_JACOBIAN = 0x02,
  PPS_TIMESTEP = 0x04,
  PPS_NEWTONIT = 0x08
};

enum CouplingType
{
  COUPLING_DIAG,
  COUPLING_FULL,
  COUPLING_CUSTOM
};

enum DGResidualType
{
  Element,
  Neighbor
};

enum DGJacobianType
{
  ElementElement,
  ElementNeighbor,
  NeighborElement,
  NeighborNeighbor
};

enum ConstraintType
{
  Slave = Element,
  Master = Neighbor
};

enum ConstraintJacobianType
{
  SlaveSlave = ElementElement,
  SlaveMaster = ElementNeighbor,
  MasterSlave = NeighborElement,
  MasterMaster = NeighborNeighbor
};

enum CoordinateSystemType
{
  COORD_XYZ,
  COORD_RZ
};

enum PPSOutputType
{
  PPS_OUTPUT_NONE,
  PPS_OUTPUT_SCREEN,
  PPS_OUTPUT_FILE,
  PPS_OUTPUT_BOTH
};

const SubdomainID ANY_BLOCK_ID = (SubdomainID) -1;

const BoundaryID ANY_BOUNDARY_ID = (BoundaryID) -1;

/* Wrappers for extern random number generator */
inline void seed(unsigned int s)
{
  mt_seed32new(s);
}

inline double rand()
{
  // For now we will try the 64-bit values
  return mt_ldrand();
}

} // namespace Moose


#define LENGTHOF(a) (sizeof(a)/sizeof(a[0]))

#endif /* MOOSE_H */
