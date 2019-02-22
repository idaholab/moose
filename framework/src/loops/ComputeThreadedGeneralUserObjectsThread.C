//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeThreadedGeneralUserObjectsThread.h"

#include "MooseApp.h"
#include "Executioner.h"

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
  _fe_problem.getMooseApp().getExecutioner()->setException(what);
}

void
ComputeThreadedGeneralUserObjectsThread::operator()(const GeneralUserObjectRange & range)
{
  try
  {
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
