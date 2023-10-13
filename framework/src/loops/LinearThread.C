//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearThread.h"
#include "LinearSystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "KernelBase.h"
#include "IntegratedBCBase.h"
#include "DGKernelBase.h"
#include "InterfaceKernelBase.h"
#include "Material.h"
#include "TimeKernel.h"
#include "SwapBackSentinel.h"
#include "FVTimeKernel.h"
#include "ComputeJacobianThread.h"

#include "libmesh/threads.h"

LinearThread::LinearThread(FEProblemBase & fe_problem)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _linear_system(fe_problem.currentLinearSystem()),
    _has_active_objects(false)
{
}

// Splitting Constructor
LinearThread::LinearThread(LinearThread & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _linear_system(x._linear_system),
    _has_active_objects(x._has_active_objects)
{
}

LinearThread::~LinearThread() {}

void
LinearThread::operator()(const ConstElemRange & range, bool bypass_threading)
{
  if (_has_active_objects)
    ThreadedElementLoop<ConstElemRange>::operator()(range, bypass_threading);
}

void
LinearThread::subdomainChanged()
{
  // This should come first to setup the residual objects before we do dependency determination of
  // material properties and variables
  determineObjectWarehouses();

  _fe_problem.subdomainSetup(_subdomain, _tid);
}

void
LinearThread::onElement(const Elem * /*elem*/)
{
}

void
LinearThread::computeOnElement()
{
}

void
LinearThread::onBoundary(const Elem * /*elem*/,
                         unsigned int /*side*/,
                         BoundaryID /*bnd_id*/,
                         const Elem * /*lower_d_elem=nullptr*/)
{
}

void
LinearThread::computeOnBoundary(BoundaryID /*bnd_id*/, const Elem * /*lower_d_elem*/)
{
}

void
LinearThread::onInterface(const Elem * /*elem*/, unsigned int /*side*/, BoundaryID /*bnd_id*/)
{
}

void
LinearThread::computeOnInterface(BoundaryID /*bnd_id*/)
{
}

void
LinearThread::onInternalSide(const Elem * /*elem*/, unsigned int /*side*/)
{
}

void
LinearThread::computeOnInternalFace(const Elem * /*neighbor*/)
{
}

void
LinearThread::compute(FVElementalKernel & kernel)
{
  compute(static_cast<ResidualObject &>(kernel));
}

void
LinearThread::postElement(const Elem * /*elem*/)
{
}

void
LinearThread::post()
{
}
