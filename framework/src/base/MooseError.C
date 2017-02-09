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

#include "MooseError.h"
#include "MooseUtils.h"


namespace moose
{
namespace internal
{

std::string
mooseMsgFmt(const std::string & msg, const std::string & title, const std::string & color)
{
  std::ostringstream oss;
  oss << "\n\n"
      << color
      << "\n\n" << title << "\n"
      << msg
      << COLOR_DEFAULT
      << "\n\n";
  return oss.str();
}

static Threads::spin_mutex moose_err_lock;

[[ noreturn ]] void
mooseErrorRaw(std::string msg, const std::string prefix)
{
  msg = mooseMsgFmt(msg, "*** ERROR ***", COLOR_RED);

  if (Moose::_throw_on_error)
  {
    if (!prefix.empty())
      MooseUtils::indentMessage(prefix, msg);
    throw std::runtime_error(msg);
  }

  std::ostringstream oss;
  oss << msg << "\n";

  if (libMesh::global_n_processors() == 1)
    print_trace(oss);

  msg = oss.str();
  if (!prefix.empty())
    MooseUtils::indentMessage(prefix, msg);

  Threads::spin_mutex::scoped_lock lock(moose_err_lock);

  Moose::err << msg << std::flush;

  if (libMesh::global_n_processors() > 1)
    libMesh::write_traceout();

  MOOSE_ABORT;
}

void
mooseStreamAll(std::ostringstream &) { }

} // namespace internal
} // namespace moose
