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

// libMesh includes
#include "libmesh/print_trace.h"
#include "libmesh/libmesh_common.h"

// C++ includes
#include <cstdlib>

/**
 * Application abort macro. Uses MPI_Abort if available, std::abort otherwise
 */
#if defined(LIBMESH_HAVE_MPI)
#define MOOSE_ABORT do { MPI_Abort(libMesh::GLOBAL_COMM_WORLD, 1); std::abort(); } while (0)
#else
#define MOOSE_ABORT do { std::abort(); } while (0)
#endif

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
      MOOSE_ABORT;                                                                  \
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
      std::ostringstream _assert_oss_;                                              \
      _assert_oss_                                                                  \
        << (Moose::_color_console ? XTERM_RED : "")                                 \
        << "\n\nAssertion `" #asserted "' failed\n"                                 \
        << msg                                                                      \
        << "\nat "                                                                  \
        << __FILE__ << ", line " << __LINE__                                        \
        << (Moose::_color_console ? XTERM_DEFAULT : "")                             \
        << std::endl;                                                               \
      if (Moose::_throw_on_error)                                                   \
        throw std::runtime_error(_assert_oss_.str());                               \
      else                                                                          \
      {                                                                             \
        Moose::err << _assert_oss_.str() << std::flush;                             \
        if (libMesh::global_n_processors() == 1)                                    \
          print_trace();                                                            \
        else                                                                        \
          libMesh::write_traceout();                                                \
        libmesh_here();                                                             \
        MOOSE_ABORT;                                                                \
      }                                                                             \
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
        << "\n\n*** Warning ***\n"                                                  \
        << msg                                                                      \
        << "\nat " << __FILE__ << ", line " << __LINE__                             \
        << (Moose::_color_console ? XTERM_DEFAULT : "")                             \
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
      );                                                                            \
    } while (0)

#define mooseDeprecated(msg)                                                                                \
  do                                                                                                        \
  {                                                                                                         \
    if (Moose::_deprecated_is_error)                                                                        \
      mooseError("\n\nDeprecated code:\n" << msg << '\n');                                                  \
    else                                                                                                    \
      mooseDoOnce(                                                                                          \
        Moose::out                                                                                          \
          << (Moose::_color_console ? XTERM_YELLOW : "")                                                    \
          << "*** Warning, This code is deprecated, and likely to be removed in future library versions!\n" \
          << msg << '\n'                                                                                    \
          << __FILE__ << ", line " << __LINE__ << ", compiled "                                             \
          << LIBMESH_DATE << " at " << LIBMESH_TIME << " ***"                                               \
          << (Moose::_color_console ? XTERM_DEFAULT : "")                                                   \
          << std::endl;                                                                                     \
      );                                                                                                    \
   } while (0)

#define mooseCheckMPIErr(err) do { if (err != MPI_SUCCESS) { if (libMesh::global_n_processors() == 1) print_trace(); libmesh_here(); MOOSE_ABORT; } } while (0)

#endif /* MOOSEERRORS_H */
