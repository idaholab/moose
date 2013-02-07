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

#ifndef MOOSEERROR_H
#define MOOSEERROR_H

// libMesh includes
//#include "libmesh/libmesh_config.h"
#include "libmesh/print_trace.h"
#include "libmesh/libmesh_common.h"
//#include "libmesh/perf_log.h"
//#include "libmesh/parallel.h"

#include <string>

/**
 * MOOSE wrapped versions of useful libMesh macros (see libmesh_common.h)
 */
#define mooseError(msg) do { std::cerr << "\n\n" << msg << "\n\n"; if (libMesh::n_processors() == 1) print_trace(); libmesh_here(); MPI_Abort(libMesh::COMM_WORLD,1); exit(1); } while(0)

#ifdef NDEBUG
#define mooseAssert(asserted, msg)
#else
#define mooseAssert(asserted, msg)  do { if (!(asserted)) { std::cerr << "\n\nAssertion `" #asserted "' failed\n" << msg << "\nat " << __FILE__ << ", line " << __LINE__ << std::endl; if (libMesh::n_processors() == 1) print_trace(); libmesh_here(); MPI_Abort(libMesh::COMM_WORLD,1); } } while(0)
#endif

#define mooseWarning(msg) do { std::cerr << "\n\n*** Warning ***\n" << msg << "\nat " << __FILE__ << ", line " << __LINE__ << "\n" << std::endl; } while(0)

#define mooseDoOnce(do_this) do { static bool did_this_already = false; if (!did_this_already) { did_this_already = true; do_this; } } while (0)

#define mooseDeprecated() mooseDoOnce(std::cout << "*** Warning, This code is deprecated, and likely to be removed in future library versions! " << __FILE__ << ", line " << __LINE__ << ", compiled " << __DATE__ << " at " << __TIME__ << " ***" << std::endl;)


#endif /* MOOSEERRORS_H */
