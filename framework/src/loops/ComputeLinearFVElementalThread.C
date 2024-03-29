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
  : _fe_problem(fe_problem),
    _linear_system_number(linear_system_num),
    _mode(mode),
    _tags(tags),
    _subdomain(Moose::INVALID_BLOCK_ID),
    _old_subdomain(Moose::INVALID_BLOCK_ID)
{
}

// Splitting Constructor
ComputeLinearFVElementalThread::ComputeLinearFVElementalThread(ComputeLinearFVElementalThread & x,
                                                               Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _linear_system_number(x._linear_system_number),
    _mode(x._mode),
    _tags(x._tags),
    _subdomain(x._subdomain),
    _old_subdomain(x._old_subdomain)
{
}

void
ComputeLinearFVElementalThread::operator()(const ElemInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  _old_subdomain = Moose::INVALID_BLOCK_ID;

  printGeneralExecutionInformation();

  // Iterate over all the elements in the range
  for (const auto & elem_info : range)
  {
    _subdomain = elem_info->subdomain_id();
    if (_subdomain != _old_subdomain)
    {
      fetchSystemContributionObjects();
      printBlockExecutionInformation();
    }

    const Real elem_volume = elem_info->volume() * elem_info->coordFactor();
    for (auto kernel : _fv_kernels)
    {
      kernel->setCurrentElemInfo(elem_info);
      kernel->setCurrentElemVolume(elem_volume);
      if (_mode == Moose::FV::LinearFVComputationMode::Matrix ||
          _mode == Moose::FV::LinearFVComputationMode::FullSystem)
        kernel->addMatrixContribution();
      if (_mode == Moose::FV::LinearFVComputationMode::RHS ||
          _mode == Moose::FV::LinearFVComputationMode::FullSystem)
        kernel->addRightHandSideContribution();
    }
  }
}

void
ComputeLinearFVElementalThread::join(const ComputeLinearFVElementalThread & /*y*/)
{
}

void
ComputeLinearFVElementalThread::fetchSystemContributionObjects()
{
  const auto system_number = _fe_problem.getLinearSystem(_linear_system_number).number();
  _fv_kernels.clear();
  std::vector<LinearFVElementalKernel *> kernels;
  auto base_query = _fe_problem.theWarehouse()
                        .query()
                        .template condition<AttribSysNum>(system_number)
                        .template condition<AttribSubdomains>(_subdomain)
                        .template condition<AttribSystem>("LinearFVElementalKernel")
                        .template condition<AttribThread>(_tid);

  if (_mode == Moose::FV::LinearFVComputationMode::Matrix)
    base_query.condition<AttribMatrixTags>(_tags).queryInto(kernels);
  else
    base_query.condition<AttribVectorTags>(_tags).queryInto(kernels);
  _old_subdomain = _subdomain;

  _fv_kernels = std::vector<LinearFVElementalKernel *>(kernels.begin(), kernels.end());
}

void
ComputeLinearFVElementalThread::printGeneralExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid))
    return;
  auto & console = _fe_problem.console();
  auto execute_on = _fe_problem.getCurrentExecuteOnFlag();
  console << "[DBG] Beginning linear finite volume elemental objects loop on " << execute_on
          << std::endl;
  mooseDoOnce(console << "[DBG] Loop on elements (ElemInfo), objects ordered on each face: "
                      << std::endl;
              console << "[DBG] - linear finite volume kernels" << std::endl;);
}

void
ComputeLinearFVElementalThread::printBlockExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid) || !_fv_kernels.size())
    return;

  auto & console = _fe_problem.console();
  console << "[DBG] Linear FV elemental kernels on block "
          << _fe_problem.mesh().getSubdomainName(_subdomain);

  // Print the list of objects
  std::vector<MooseObject *> kernels_to_print;
  for (const auto & kernel : _fv_kernels)
    kernels_to_print.push_back(dynamic_cast<MooseObject *>(kernel));
  console << ConsoleUtils::formatString(ConsoleUtils::mooseObjectVectorToString(kernels_to_print),
                                        "[DBG]")
          << std::endl;
}
