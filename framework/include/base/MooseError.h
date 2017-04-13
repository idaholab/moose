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
#include "libmesh/threads.h"

// libMesh includes
#include "libmesh/print_trace.h"
#include "libmesh/libmesh_common.h"

// C++ includes
#include <cstdlib>

/// Application abort macro. Uses MPI_Abort if available, std::abort otherwise
#if defined(LIBMESH_HAVE_MPI)
#define MOOSE_ABORT                                                                                \
  do                                                                                               \
  {                                                                                                \
    MPI_Abort(libMesh::GLOBAL_COMM_WORLD, 1);                                                      \
    std::abort();                                                                                  \
  } while (0)
#else
#define MOOSE_ABORT                                                                                \
  do                                                                                               \
  {                                                                                                \
    std::abort();                                                                                  \
  } while (0)
#endif

#define mooseDoOnce(do_this)                                                                       \
  do                                                                                               \
  {                                                                                                \
    static bool did_this_already = false;                                                          \
    if (!did_this_already)                                                                         \
    {                                                                                              \
      did_this_already = true;                                                                     \
      do_this;                                                                                     \
    }                                                                                              \
  } while (0)

#define mooseCheckMPIErr(err)                                                                      \
  do                                                                                               \
  {                                                                                                \
    if (err != MPI_SUCCESS)                                                                        \
    {                                                                                              \
      if (libMesh::global_n_processors() == 1)                                                     \
        print_trace();                                                                             \
      libmesh_here();                                                                              \
      MOOSE_ABORT;                                                                                 \
    }                                                                                              \
  } while (0)

#define mooseException(msg)                                                                        \
  do                                                                                               \
  {                                                                                                \
    std::ostringstream _exception_oss_;                                                            \
    _exception_oss_ << msg;                                                                        \
                                                                                                   \
    throw MooseException(_exception_oss_.str());                                                   \
  } while (0)

#ifdef NDEBUG
#define mooseAssert(asserted, msg)
#else
#define mooseAssert(asserted, msg)                                                                 \
  do                                                                                               \
  {                                                                                                \
    if (!(asserted))                                                                               \
    {                                                                                              \
      std::ostringstream _assert_oss_;                                                             \
      _assert_oss_ << COLOR_RED << "\n\nAssertion `" #asserted "' failed\n"                        \
                   << msg << "\nat " << __FILE__ << ", line " << __LINE__ << COLOR_DEFAULT         \
                   << std::endl;                                                                   \
      if (Moose::_throw_on_error)                                                                  \
        throw std::runtime_error(_assert_oss_.str());                                              \
      else                                                                                         \
      {                                                                                            \
        Moose::err << _assert_oss_.str() << std::flush;                                            \
        if (libMesh::global_n_processors() == 1)                                                   \
          print_trace();                                                                           \
        else                                                                                       \
          libMesh::write_traceout();                                                               \
        libmesh_here();                                                                            \
        MOOSE_ABORT;                                                                               \
      }                                                                                            \
    }                                                                                              \
  } while (0)
#endif

template <typename... Args>
[[noreturn]] void mooseError(Args &&... args);

namespace moose
{
namespace internal
{

std::string
mooseMsgFmt(const std::string & msg, const std::string & title, const std::string & color);

[[noreturn]] void mooseErrorRaw(std::string msg, const std::string prefix = "");

void mooseStreamAll(std::ostringstream & ss);

template <typename T, typename... Args>
void
mooseStreamAll(std::ostringstream & ss, T && val, Args &&... args)
{
  ss << val;
  mooseStreamAll(ss, std::forward<Args>(args)...);
}

template <typename S, typename... Args>
void
mooseWarningStream(S & oss, Args &&... args)
{
  if (Moose::_warnings_are_errors)
    mooseError(std::forward<Args>(args)...);

  std::ostringstream ss;
  mooseStreamAll(ss, args...);
  std::string msg = mooseMsgFmt(ss.str(), "*** Warning ***", COLOR_YELLOW);
  if (Moose::_throw_on_error)
    throw std::runtime_error(msg);

  oss << msg << std::flush;
}

template <typename S, typename... Args>
void
mooseInfoStream(S & oss, Args &&... args)
{
  mooseDoOnce({
    std::ostringstream ss;
    mooseStreamAll(ss, args...);
    std::string msg = mooseMsgFmt(ss.str(), "*** Info ***", COLOR_CYAN);
    oss << msg << std::flush;
  });
}

template <typename S, typename... Args>
void
mooseDeprecatedStream(S & oss, Args &&... args)
{
  if (Moose::_deprecated_is_error)
    mooseError("\n\nDeprecated code:\n", std::forward<Args>(args)...);

  mooseDoOnce(std::ostringstream ss; mooseStreamAll(ss, args...);
              std::string msg = mooseMsgFmt(
                  ss.str(),
                  "*** Warning, This code is deprecated and will be removed in future versions!",
                  COLOR_YELLOW);
              oss << msg;
              ss.str("");
              if (libMesh::global_n_processors() == 1) print_trace(ss);
              else libMesh::write_traceout();
              oss << ss.str() << std::endl;);
}

} // namespace internal
} // namespace moose

/// Emit an error message with the given stringified, concatenated args and
/// terminate the application.  Inside static functions, you will need to
/// explicitly scope your mooseError call - i.e. do "::mooseError(arg1, ...);".
template <typename... Args>
[[noreturn]] void
mooseError(Args &&... args)
{
  std::ostringstream oss;
  moose::internal::mooseStreamAll(oss, std::forward<Args>(args)...);
  moose::internal::mooseErrorRaw(oss.str());
}

/// Emit a warning message with the given stringified, concatenated args.
/// Inside static functions, you will need to explicitly scope your
/// mooseWarning call - i.e. do "::mooseWarning(arg1, ...);".
template <typename... Args>
void
mooseWarning(Args &&... args)
{
  moose::internal::mooseWarningStream(Moose::out, std::forward<Args>(args)...);
}

/// Emit a deprecated code/feature message with the given stringified, concatenated args.
template <typename... Args>
void
mooseDeprecated(Args &&... args)
{
  moose::internal::mooseDeprecatedStream(Moose::out, std::forward<Args>(args)...);
}

/// Emit an informational message with the given stringified, concatenated args.
template <typename... Args>
void
mooseInfo(Args &&... args)
{
  moose::internal::mooseInfoStream(Moose::out, std::forward<Args>(args)...);
}

#endif /* MOOSEERRORS_H */
