#ifndef MOOSE_H_
#define MOOSE_H_

// libMesh includes
#include "print_trace.h"
#include "libmesh_common.h"
#include "perf_log.h"
#include "mtwist.h"

typedef Real                     PostprocessorValue;

/**
 * A function to call when you need the whole program to die and spit out a message
 */
#ifndef NDEBUG
#define mooseError(msg) do { std::cerr << "\n\n" << msg << "\n\n"; print_trace(); libmesh_error(); } while(0)
#else
#define mooseError(msg) do { std::cerr << "\n\n" << msg << "\n\n"; libmesh_error(); } while(0)
#endif

#ifdef NDEBUG
#define mooseAssert(asserted, msg)
#else
#define mooseAssert(asserted, msg)  do { if (!(asserted)) { std::cerr << "\n\nAssertion `" #asserted "' failed.\n" << msg << std::endl; libmesh_error(); } } while(0)
#endif

#define mooseWarning(msg) do { std::cerr << "\n\n*** Warning ***\n" << msg << "\nat " << __FILE__ << ", line " << __LINE__ << "\n" << std::endl; } while(0)


namespace Moose
{

class ImplicitSystem;
class ActionWarehouse;

extern ActionWarehouse action_warehouse;
/**
 * Perflog to be used by applications.
 * If the application prints this in the end they will get performance info.
 */
extern PerfLog perf_log;

/**
 * Register objects that are in MOOSE 
 */
void registerObjects();
void addActionTypes();
void registerActions();

void setSolverDefaults(ImplicitSystem & system);

/**
 * Framework-wide stuff
 */

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

const unsigned int ANY_BLOCK_ID = (unsigned int) -1;

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

} // namespace

#endif /* MOOSE_H_ */
