//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IntegralRayKernelBase.h"

// MOOSE includes
#include "Assembly.h"
#include "NonlinearSystemBase.h"

InputParameters
IntegralRayKernelBase::validParams()
{
  auto params = RayKernelBase::validParams();

  // Set so that in the case that a derived class doesn't have any coupled
  // variables or materials that the RayTracingStudy knows that it still needs
  // qps and weights to be reinit on its segment
  params.set<bool>("_need_segment_reinit") = true;

  return params;
}

IntegralRayKernelBase::IntegralRayKernelBase(const InputParameters & params)
  : RayKernelBase(params),
    _assembly(_fe_problem.assembly(_tid)),
    _q_point(_assembly.qPoints()),
    _JxW(_assembly.JxW())
{
}

void
IntegralRayKernelBase::preExecuteStudy()
{
  mooseAssert(needSegmentReinit(), "Must be true");
}
