//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLinearFVElementalThread.h"
#include "LinearSystem.h"
#include "LinearFVKernel.h"
#include "LinearFVElementalKernel.h"

ComputeLinearFVElementalThread::ComputeLinearFVElementalThread(
    FEProblemBase & fe_problem,
    const unsigned int linear_system_num,
    const Moose::FV::LinearFVComputationMode mode,
    const std::set<TagID> & tags)
  : _fe_problem(fe_problem), _linear_system_number(linear_system_num), _mode(mode), _tags(tags)
{
}

// Splitting Constructor
ComputeLinearFVElementalThread::ComputeLinearFVElementalThread(ComputeLinearFVElementalThread & x,
                                                               Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _linear_system_number(x._linear_system_number),
    _mode(x._mode),
    _tags(x._tags)
{
}

void
ComputeLinearFVElementalThread::operator()(const ElemInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  std::vector<LinearFVElementalKernel *> kernels;
  auto base_query =
      _fe_problem.theWarehouse()
          .query()
          .condition<AttribSysNum>(_fe_problem.getLinearSystem(_linear_system_number).number())
          .condition<AttribSystem>("LinearFVElementalKernel")
          .condition<AttribThread>(_tid);

  if (_mode == Moose::FV::LinearFVComputationMode::Matrix)
    base_query.condition<AttribMatrixTags>(_tags).queryInto(kernels);
  else
    base_query.condition<AttribVectorTags>(_tags).queryInto(kernels);

  // Iterate over all the elements in the range
  for (const auto & elem_info : range)
    for (auto kernel : kernels)
    {
      kernel->setCurrentElemInfo(elem_info);
      if (_mode == Moose::FV::LinearFVComputationMode::Matrix ||
          _mode == Moose::FV::LinearFVComputationMode::FullSystem)
        kernel->addMatrixContribution();
      if (_mode == Moose::FV::LinearFVComputationMode::RHS ||
          _mode == Moose::FV::LinearFVComputationMode::FullSystem)
        kernel->addRightHandSideContribution();
    }
}

void
ComputeLinearFVElementalThread::join(const ComputeLinearFVElementalThread & /*y*/)
{
}
