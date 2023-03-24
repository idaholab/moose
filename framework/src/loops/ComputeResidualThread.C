//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeResidualThread.h"
#include "KernelBase.h"
#include "DGKernelBase.h"
#include "IntegratedBCBase.h"
#include "FVElementalKernel.h"
#include "InterfaceKernelBase.h"
#include "libmesh/threads.h"

ComputeResidualThread::ComputeResidualThread(FEProblemBase & fe_problem,
                                             const std::set<TagID> & tags)
  : NonlinearThread(fe_problem), _tags(tags)
{
}

// Splitting Constructor
ComputeResidualThread::ComputeResidualThread(ComputeResidualThread & x, Threads::split split)
  : NonlinearThread(x, split), _tags(x._tags)
{
}

ComputeResidualThread::~ComputeResidualThread() {}

void
ComputeResidualThread::compute(ResidualObject & ro)
{
  ro.computeResidual();
}

void
ComputeResidualThread::accumulateLower()
{
  _fe_problem.addResidualLower(_tid);
}

void
ComputeResidualThread::accumulateNeighbor()
{
  _fe_problem.addResidualNeighbor(_tid);
}

void
ComputeResidualThread::accumulateNeighborLower()
{
  _fe_problem.addResidualNeighbor(_tid);
  _fe_problem.addResidualLower(_tid);
}

void
ComputeResidualThread::accumulate()
{
  _fe_problem.cacheResidual(_tid);
  _num_cached++;

  if (_num_cached % 20 == 0)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _fe_problem.addCachedResidual(_tid);
  }
}

void
ComputeResidualThread::join(const ComputeResidualThread & /*y*/)
{
}

void
ComputeResidualThread::determineResidualObjects()
{
  // If users pass a empty vector or a full size of vector,
  // we take all kernels
  if (!_tags.size() || _tags.size() == _fe_problem.numVectorTags(Moose::VECTOR_TAG_RESIDUAL))
  {
    _tag_kernels = &_kernels;
    _dg_warehouse = &_dg_kernels;
    _ibc_warehouse = &_integrated_bcs;
    _ik_warehouse = &_interface_kernels;
  }
  // If we have one tag only,
  // We call tag based storage
  else if (_tags.size() == 1)
  {
    _tag_kernels = &(_kernels.getVectorTagObjectWarehouse(*(_tags.begin()), _tid));
    _dg_warehouse = &(_dg_kernels.getVectorTagObjectWarehouse(*(_tags.begin()), _tid));
    _ibc_warehouse = &(_integrated_bcs.getVectorTagObjectWarehouse(*(_tags.begin()), _tid));
    _ik_warehouse = &(_interface_kernels.getVectorTagObjectWarehouse(*(_tags.begin()), _tid));
  }
  // This one may be expensive
  else
  {
    _tag_kernels = &(_kernels.getVectorTagsObjectWarehouse(_tags, _tid));
    _dg_warehouse = &(_dg_kernels.getVectorTagsObjectWarehouse(_tags, _tid));
    _ibc_warehouse = &(_integrated_bcs.getVectorTagsObjectWarehouse(_tags, _tid));
    _ik_warehouse = &(_interface_kernels.getVectorTagsObjectWarehouse(_tags, _tid));
  }

  if (_fe_problem.haveFV())
  {
    _fv_kernels.clear();
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribSystem>("FVElementalKernel")
        .template condition<AttribSubdomains>(_subdomain)
        .template condition<AttribThread>(_tid)
        .template condition<AttribVectorTags>(_tags)
        .queryInto(_fv_kernels);
  }
}

void
ComputeResidualThread::printGeneralExecutionInformation() const
{
  if (_fe_problem.shouldPrintExecution(_tid))
  {
    const auto & console = _fe_problem.console();
    const auto execute_on = _fe_problem.getCurrentExecuteOnFlag();
    console << "[DBG] Beginning elemental loop to compute residual on " << execute_on << std::endl;
    mooseDoOnce(
        console << "[DBG] Execution order on each element:" << std::endl;
        console << "[DBG] - kernels on element quadrature points" << std::endl;
        console << "[DBG] - finite volume elemental kernels on element" << std::endl;
        console << "[DBG] - integrated boundary conditions on element side quadrature points"
                << std::endl;
        console << "[DBG] - DG kernels on element side quadrature points" << std::endl;
        console << "[DBG] - interface kernels on element side quadrature points" << std::endl;);
  }
}

void
ComputeResidualThread::printBlockExecutionInformation() const
{
  // Number of objects executing is approximated by size of warehouses
  const int num_objects = _kernels.size() + _fv_kernels.size() + _integrated_bcs.size() +
                          _dg_kernels.size() + _interface_kernels.size();
  const auto & console = _fe_problem.console();
  if (_fe_problem.shouldPrintExecution(_tid) && num_objects > 0)
  {
    if (_blocks_exec_printed.count(_subdomain))
      return;
    console << "[DBG] Ordering of Residual Objects on block " << _subdomain << std::endl;
    if (_kernels.hasActiveBlockObjects(_subdomain, _tid))
    {
      console << "[DBG] Ordering of kernels:" << std::endl;
      console << _kernels.activeObjectsToFormattedString() << std::endl;
    }
    if (_fv_kernels.size())
    {
      console << "[DBG] Ordering of FV elemental kernels:" << std::endl;
      std::string fvkernels =
          std::accumulate(_fv_kernels.begin() + 1,
                          _fv_kernels.end(),
                          _fv_kernels[0]->name(),
                          [](const std::string & str_out, FVElementalKernel * kernel)
                          { return str_out + " " + kernel->name(); });
      console << ConsoleUtils::formatString(fvkernels, "[DBG]") << std::endl;
    }
    if (_dg_kernels.hasActiveBlockObjects(_subdomain, _tid))
    {
      console << "[DBG] Ordering of DG kernels:" << std::endl;
      console << _dg_kernels.activeObjectsToFormattedString() << std::endl;
    }
    if (_integrated_bcs.hasActiveBlockObjects(_subdomain, _tid))
    {
      console << "[DBG] Ordering of boundary conditions:" << std::endl;
      console << _integrated_bcs.activeObjectsToFormattedString() << std::endl;
    }
    if (_interface_kernels.hasActiveBlockObjects(_subdomain, _tid))
    {
      console << "[DBG] Ordering of interface kernels:" << std::endl;
      console << _interface_kernels.activeObjectsToFormattedString() << std::endl;
    }
  }
  else if (_fe_problem.shouldPrintExecution(_tid) && num_objects == 0 &&
           !_blocks_exec_printed.count(_subdomain))
    console << "[DBG] No Active Residual Objects on block " << _subdomain << std::endl;

  _blocks_exec_printed.insert(_subdomain);
}
