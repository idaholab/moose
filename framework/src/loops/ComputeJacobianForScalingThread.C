//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeJacobianForScalingThread.h"
#include "MooseError.h"
#include "NonlinearSystemBase.h"

#include "libmesh/elem.h"

using namespace libMesh;

ComputeJacobianForScalingThread::ComputeJacobianForScalingThread(FEProblemBase & fe_problem,
                                                                 const std::set<TagID> & tags)
  : ComputeFullJacobianThread(fe_problem, tags)
{
}

// Splitting Constructor
ComputeJacobianForScalingThread::ComputeJacobianForScalingThread(
    ComputeJacobianForScalingThread & x, Threads::split split)
  : ComputeFullJacobianThread(x, split)
{
}

void
ComputeJacobianForScalingThread::operator()(const ConstElemRange & range,
                                            bool bypass_threading /*= false*/)
{
  try
  {
    try
    {
      ParallelUniqueId puid;
      _tid = bypass_threading ? 0 : puid.id;

      pre();

      _subdomain = Moose::INVALID_BLOCK_ID;
      _neighbor_subdomain = Moose::INVALID_BLOCK_ID;
      typename ConstElemRange::const_iterator el = range.begin();
      for (el = range.begin(); el != range.end(); ++el)
      {
        if (!keepGoing())
          break;

        const Elem * elem = *el;

        preElement(elem);

        _old_subdomain = _subdomain;
        _subdomain = elem->subdomain_id();
        if (_subdomain != _old_subdomain)
          subdomainChanged();

        onElement(elem);

        postElement(elem);
      } // range

      post();
    }
    catch (libMesh::LogicError & e)
    {
      mooseException("We caught a libMesh error in ComputeJacobianForScalingThread: ", e.what());
    }
    catch (MetaPhysicL::LogicError & e)
    {
      moose::translateMetaPhysicLError(e);
    }
  }
  catch (MooseException & e)
  {
    caughtMooseException(e);
  }
}

void
ComputeJacobianForScalingThread::computeOnElement()
{
  if (_nl.offDiagonalsInAutoScaling())
    ComputeFullJacobianThread::computeOnElement();
  else
    ComputeJacobianThread::computeOnElement();
}
