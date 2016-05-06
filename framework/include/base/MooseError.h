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

#include "Moose.h"
#include "MooseException.h"

// temporary fix to allow merging moose PR #4278 until libmesh PR #415 is merged
#ifndef __LIBMESH_TIME__
#  define __LIBMESH_TIME__ __TIME__
#endif
#ifndef __LIBMESH_DATE__
#  define __LIBMESH_DATE__ __DATE__
#endif

// libMesh includes
#include "libmesh/print_trace.h"

/**
 * MOOSE wrapped versions of useful libMesh macros (see libmesh_common.h)
 */
#define mooseError(msg)                                                             \
  do                                                                                \
  {                                                                                 \
    std::ostringstream _error_oss_;                                                 \
    _error_oss_ << "\n\n"                                                           \
                << (Moose::_color_console ? XTERM_RED : "")                         \
                << "\n\n*** ERROR ***\n"                                            \
                << msg                                                              \
                << (Moose::_color_console ? XTERM_DEFAULT : "")                     \
                << "\n\n";                                                          \
    if (Moose::_throw_on_error)                                                     \
      throw std::runtime_error(_error_oss_.str());                                  \
    else                                                                            \
    {                                                                               \
      Moose::err << _error_oss_.str() << std::flush;                                \
      if (libMesh::global_n_processors() == 1)                                      \
        print_trace();                                                              \
      else                                                                          \
        libMesh::write_traceout();                                                  \
      libmesh_here();                                                               \
      MPI_Abort(libMesh::GLOBAL_COMM_WORLD,1);                                      \
      exit(1);                                                                      \
    }                                                                               \
  } while (0)


#define mooseException(msg)                                                         \
  do                                                                                \
  {                                                                                 \
    std::ostringstream _exception_oss_;                                             \
    _exception_oss_ << msg;                                                         \
                                                                                    \
    throw MooseException(_exception_oss_.str());                                    \
  } while (0)


#ifdef NDEBUG
#define mooseAssert(asserted, msg)
#else
#define mooseAssert(asserted, msg)                                                  \
  do                                                                                \
  {                                                                                 \
    if (!(asserted))                                                                \
    {                                                                               \
      Moose::err                                                                    \
        << (Moose::_color_console ? XTERM_RED : "")                                 \
        << "\n\nAssertion `" #asserted "' failed\n"                                 \
        << msg                                                                      \
        << "\nat "                                                                  \
        << __FILE__ << ", line " << __LINE__                                        \
        << (Moose::_color_console ? XTERM_DEFAULT : "")                             \
        << std::endl;                                                               \
     if (libMesh::global_n_processors() == 1)                                       \
       print_trace();                                                               \
     else                                                                           \
       libMesh::write_traceout();                                                   \
     libmesh_here();                                                                \
     MPI_Abort(libMesh::GLOBAL_COMM_WORLD,1);                                       \
     exit(1);                                                                       \
    }                                                                               \
  } while (0)
#endif

#define mooseWarning(msg)                                                           \
  do                                                                                \
  {                                                                                 \
    if (Moose::_warnings_are_errors)                                                \
      mooseError(msg);                                                              \
    else                                                                            \
    {                                                                               \
      std::ostringstream _warn_oss_;                                                \
                                                                                    \
      _warn_oss_                                                                    \
        << (Moose::_color_console ? XTERM_YELLOW : "")                              \
      << "\n\n*** Warning ***\n"                                                    \
      << msg                                                                        \
      << "\nat " << __FILE__ << ", line " << __LINE__                               \
      << (Moose::_color_console ? XTERM_DEFAULT : "")                               \
        << "\n\n";                                                                  \
      if (Moose::_throw_on_error)                                                   \
        throw std::runtime_error(_warn_oss_.str());                                 \
      else                                                                          \
        Moose::err << _warn_oss_.str() << std::flush;                               \
    }                                                                               \
  } while (0)

#define mooseDoOnce(do_this) do { static bool did_this_already = false; if (!did_this_already) { did_this_already = true; do_this; } } while (0)

#define mooseInfo(msg)                                                              \
    do                                                                              \
    {                                                                               \
      mooseDoOnce(                                                                  \
        {                                                                           \
          Moose::out                                                                \
            << (Moose::_color_console ? XTERM_CYAN : "")                            \
            << "\n\n*** Info ***\n"                                                 \
            << msg                                                                  \
            << "\nat " << __FILE__ << ", line " << __LINE__                         \
            << (Moose::_color_console ? XTERM_DEFAULT : "")                         \
            << "\n" << std::endl;                                                   \
        }                                                                           \
        );                                                                          \
    } while (0)

#define mooseDeprecated(msg)                                                                                \
  do                                                                                                        \
  {                                                                                                         \
    if (Moose::_warnings_are_errors || Moose::_deprecated_is_error)                                         \
      mooseError("\n\nDeprecated code:\n" << msg << '\n');                                                  \
    else                                                                                                    \
      mooseDoOnce(                                                                                          \
        Moose::out                                                                                          \
          << (Moose::_color_console ? XTERM_YELLOW : "")                                                    \
          << "*** Warning, This code is deprecated, and likely to be removed in future library versions!\n" \
          << msg << '\n'                                                                                    \
          << __FILE__ << ", line " << __LINE__ << ", compiled "                                             \
          << __LIBMESH_DATE__ << " at " << __LIBMESH_TIME__ << " ***"                                       \
          << (Moose::_color_console ? XTERM_DEFAULT : "")                                                   \
          << std::endl;                                                                                     \
        );                                                                                                  \
   } while (0)

#define mooseCheckMPIErr(err) do { if (err != MPI_SUCCESS) { if (libMesh::global_n_processors() == 1) print_trace(); libmesh_here(); MPI_Abort(libMesh::GLOBAL_COMM_WORLD,1); exit(1); } } while (0)

#endif /* MOOSEERRORS_H */
