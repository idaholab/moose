//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeThreadedGeneralUserObjectsThread.h"

ComputeThreadedGeneralUserObjectsThread::ComputeThreadedGeneralUserObjectsThread(
    FEProblemBase & fe_problem)
  : _fe_problem(fe_problem)
{
}

ComputeThreadedGeneralUserObjectsThread::ComputeThreadedGeneralUserObjectsThread(
    ComputeThreadedGeneralUserObjectsThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem)
{
}

ComputeThreadedGeneralUserObjectsThread::~ComputeThreadedGeneralUserObjectsThread() {}

void
ComputeThreadedGeneralUserObjectsThread::caughtMooseException(MooseException & e)
{
  Threads::spin_mutex::scoped_lock lock(threaded_general_user_objects_mutex);

  std::string what(e.what());
  _fe_problem.setException(what);
}

void
ComputeThreadedGeneralUserObjectsThread::operator()(const GeneralUserObjectRange & range)
{
  try
  {
    printGeneralExecutionInformation(range);
    for (auto it = range.begin(); it != range.end(); ++it)
    {
      auto & tguo = *it;
      tguo->execute();
    }
  }
  catch (MooseException & e)
  {
    caughtMooseException(e);
  }
}

void
ComputeThreadedGeneralUserObjectsThread::printGeneralExecutionInformation(
    const GeneralUserObjectRange & range) const
{
  // TODO: Threaded UOs dont know their thread number so this will print too often
  if (!_fe_problem.shouldPrintExecution(0) || !range.size())
    return;

  const auto & console = _fe_problem.console();
  const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
  console << "[DBG] Executing Threaded General User Object " << (*range.begin())->name() << " on "
          << execute_on << std::endl;
}
