//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GatherRCDataElementThread.h"
#include "INSFVAttributes.h"
#include "INSFVMomentumResidualObject.h"

GatherRCDataElementThread::GatherRCDataElementThread(FEProblemBase & fe_problem,
                                                     const std::vector<unsigned int> & vars)
  : ThreadedElementLoop<ConstElemRange>(fe_problem), _vars(vars)
{
}

// Splitting Constructor
GatherRCDataElementThread::GatherRCDataElementThread(GatherRCDataElementThread & x,
                                                     Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split), _vars(x._vars)
{
}

void
GatherRCDataElementThread::subdomainChanged()
{
  ThreadedElementLoop<ConstElemRange>::subdomainChanged();

  _insfv_elemental_kernels.clear();

  auto queries = _fe_problem.theWarehouse()
                     .query()
                     .template condition<AttribSystem>("FVElementalKernel")
                     .template condition<AttribThread>(_tid)
                     .template condition<AttribSubdomains>(_subdomain);

  for (const auto var_num : _vars)
  {
    // We don't want to do cascading var num attributes or else the second time around we won't get
    // any results out of the query (e.g. an object cannot have a variable that simultaneously has
    // both var number 0 and 1)
    auto copied_queries = queries;
    std::vector<INSFVMomentumResidualObject *> var_eks;
    copied_queries.template condition<AttribVar>(static_cast<int>(var_num)).queryInto(var_eks);
    for (auto * const var_ek : var_eks)
      _insfv_elemental_kernels.push_back(var_ek);
  }
}

void
GatherRCDataElementThread::onElement(const Elem * const elem)
{
  mooseAssert(elem && elem->subdomain_id() == _subdomain, "sub ids don't match");

  for (auto * const insfv_ek : _insfv_elemental_kernels)
    insfv_ek->gatherRCData(*elem);
}
