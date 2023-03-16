//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"
#include "MooseUtils.h"
#include "MooseVariable.h"

#include "libmesh/string_to_enum.h"

namespace moose
{

namespace internal
{

std::string
incompatVarMsg(MooseVariableFEBase & var1, MooseVariableFEBase & var2)
{
  std::stringstream ss;
  ss << libMesh::Utility::enum_to_string<FEFamily>(var1.feType().family) << ",ORDER"
     << var1.feType().order
     << " != " << libMesh::Utility::enum_to_string<FEFamily>(var2.feType().family) << ",ORDER"
     << var2.feType().order;
  return ss.str();
}

std::string
mooseMsgFmt(const std::string & msg, const std::string & title, const std::string & color)
{
  std::ostringstream oss;
  oss << "\n" << color << "\n" << title << "\n" << msg << COLOR_DEFAULT << "\n";
  return oss.str();
}

std::string
mooseMsgFmt(const std::string & msg, const std::string & color)
{
  std::ostringstream oss;
  oss << "\n" << color << "\n" << msg << COLOR_DEFAULT << "\n";
  return oss.str();
}

[[noreturn]] void
mooseErrorRaw(std::string msg, const std::string prefix)
{
  if (Moose::_throw_on_error)
    throw std::runtime_error(msg);

  msg = mooseMsgFmt(msg, "*** ERROR ***", COLOR_RED);

  std::ostringstream oss;
  oss << msg << "\n";

  // this independent flush of the partial error message (i.e. without the
  // trace) is here because trace retrieval can be slow in some
  // circumstances, and we want to get the error message out ASAP.
  msg = oss.str();
  if (!prefix.empty())
    MooseUtils::indentMessage(prefix, msg);
  {
    Threads::spin_mutex::scoped_lock lock(moose_stream_lock);
    Moose::err << msg << std::flush;
  }

  oss.str("");
  if (Moose::show_trace && libMesh::global_n_processors() == 1)
    print_trace(oss);

  msg = oss.str();
  if (!prefix.empty())
    MooseUtils::indentMessage(prefix, msg);

  {
    Threads::spin_mutex::scoped_lock lock(moose_stream_lock);
    Moose::err << msg << std::flush;

    if (libMesh::global_n_processors() > 1)
      libMesh::write_traceout();
  }

  MOOSE_ABORT;
}

void
mooseStreamAll(std::ostringstream &)
{
}

} // namespace internal

void
translateMetaPhysicLError(const MetaPhysicL::LogicError &)
{
  mooseError(
      "We caught a MetaPhysicL error in while performing element or face loops. This is "
      "potentially due to AD not having a sufficiently large derivative container size. To "
      "increase the AD container size, you can run configure in the MOOSE root directory with the "
      "'--with-derivative-size=<n>' option and then recompile. Other causes of MetaPhysicL logic "
      "errors include evaluating functions where they are not defined or differentiable like sqrt "
      "(which gets called for vector norm functions) or log with arguments <= 0");
}

} // namespace moose
