//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"
#include "MooseUtils.h"
#include "MooseVariable.h"
#include "Registry.h"

#include "libmesh/string_to_enum.h"

using namespace libMesh;

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
mooseErrorRaw(std::string msg,
              const std::string & prefix /* = "" */,
              const hit::Node * node /* = nullptr */)
{
  if (Moose::_throw_on_error)
    throw MooseRuntimeError(msg, node);

  // Atomic that will be set as soon as any thread hits this method
  static std::atomic_flag aborting = ATOMIC_FLAG_INIT;

  // This branch will be hit after another thread has already set the atomic.
  // MPI_Abort, despite its name, does not behave like std::abort but instead
  // calls exit handlers and destroys statics. So we don't want to touch
  // anything static at this point. We'll just wait until the winning thread
  // has incurred program exit
  if (aborting.test_and_set(std::memory_order_acq_rel))
  {
    // Waiting for the other thread(s), not burning CPU
    for (;;)
      pause();
  }
  // We're the first thread to hit this method (we set the atomic), so we're
  // responsible for dumping the error and trace(s) while the remaining
  // threads wait for us to exit (via MOOSE_ABORT)
  else
  {
    // Output the message if there is one, but flush it without the trace
    // as trace retrieval can be slow in some circumstances and we want to
    // get the error message out ASAP
    if (!msg.empty())
    {
      // If we have a node available, add in the hit context (file location)
      if (node)
        msg = Moose::hitMessagePrefix(*node) + msg;

      msg = mooseMsgFmt(msg, "*** ERROR ***", COLOR_RED) + "\n";
      if (!prefix.empty()) // multiapp prefix
        MooseUtils::indentMessage(prefix, msg);

      {
        Threads::spin_mutex::scoped_lock lock(moose_stream_lock);
        Moose::err << msg << std::flush;
      }
    }

    // Print the trace if enabled and on a single rank
    if (Moose::show_trace && libMesh::global_n_processors() == 1)
    {
      std::ostringstream oss;
      print_trace(oss);
      auto trace = oss.str();
      if (!prefix.empty()) // multiapp prefix
        MooseUtils::indentMessage(prefix, trace);

      {
        Threads::spin_mutex::scoped_lock lock(moose_stream_lock);
        Moose::err << trace << std::flush;
      }
    }

    // In parallel with libMesh configured with --enable-tracefiles, this will
    // dump a trace for each rank to file
    if (libMesh::global_n_processors() > 1)
      libMesh::write_traceout();

    MOOSE_ABORT;
  }
}

void
mooseStreamAll(std::ostringstream &)
{
}

std::string
formatMooseDocumentedError(const std::string & repo_name,
                           const unsigned int issue_num,
                           const std::string & msg)
{
  const auto & repo_url = Registry::getRepositoryURL(repo_name);
  std::stringstream oss;
  oss << msg << "\n\nThis error is documented at " << repo_url << "/issues/" << issue_num << ".";
  return oss.str();
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
