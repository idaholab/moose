//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "FEProblemBase.h"

ComputeLinearFVElementalThread::ComputeLinearFVElementalThread(FEProblemBase & fe_problem,
                                                               const unsigned int system_num,
                                                               const std::set<TagID> & vector_tags,
                                                               const std::set<TagID> & matrix_tags)
  : _fe_problem(fe_problem),
    _system_number(system_num),
    _vector_tags(vector_tags),
    _matrix_tags(matrix_tags),
    _subdomain(Moose::INVALID_BLOCK_ID),
    _old_subdomain(Moose::INVALID_BLOCK_ID)
{
}

// Splitting Constructor
ComputeLinearFVElementalThread::ComputeLinearFVElementalThread(ComputeLinearFVElementalThread & x,
                                                               Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _system_number(x._system_number),
    _vector_tags(x._vector_tags),
    _matrix_tags(x._matrix_tags),
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

    // First, we execute the kernels that contribute to both the matrix and
    // right hand side
    for (auto kernel : _fv_kernels_system)
    {
      kernel->setCurrentElemInfo(elem_info);
      kernel->setCurrentElemVolume(elem_volume);
      kernel->addMatrixContribution();
      kernel->addRightHandSideContribution();
    }
    // Second, we execute the kernels that contribute to the matrix only
    for (auto kernel : _fv_kernels_matrix)
    {
      kernel->setCurrentElemInfo(elem_info);
      kernel->setCurrentElemVolume(elem_volume);
      kernel->addMatrixContribution();
    }
    // Lastly, we execute the kernels that contribute to the right hand side only
    for (auto kernel : _fv_kernels_rhs)
    {
      kernel->setCurrentElemInfo(elem_info);
      kernel->setCurrentElemVolume(elem_volume);
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
  // std::vector<LinearFVElementalKernel *> kernels_rhs;
  // std::vector<LinearFVElementalKernel *> kernels_matrix;
  auto base_query = _fe_problem.theWarehouse()
                        .query()
                        .template condition<AttribSysNum>(_system_number)
                        .template condition<AttribSubdomains>(_subdomain)
                        .template condition<AttribSystem>("LinearFVElementalKernel")
                        .template condition<AttribThread>(_tid);

  base_query.condition<AttribMatrixTags>(_matrix_tags).queryInto(_fv_kernels_matrix);
  base_query.condition<AttribVectorTags>(_vector_tags).queryInto(_fv_kernels_rhs);
  _old_subdomain = _subdomain;

  // This will remove the common elements
  MooseUtils::removeCommonSet(_fv_kernels_matrix, _fv_kernels_rhs, _fv_kernels_system);
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
  if (!_fe_problem.shouldPrintExecution(_tid) || !_fv_kernels_matrix.size() ||
      !_fv_kernels_rhs.size() || !_fv_kernels_system.size())
    return;

  auto & console = _fe_problem.console();
  console << "[DBG] Linear FV elemental kernels on block "
          << _fe_problem.mesh().getSubdomainName(_subdomain);

  // Print the list of objects
  std::vector<MooseObject *> kernels_to_print;
  for (const auto & kernel : _fv_kernels_matrix)
    kernels_to_print.push_back(dynamic_cast<MooseObject *>(kernel));
  for (const auto & kernel : _fv_kernels_rhs)
    kernels_to_print.push_back(dynamic_cast<MooseObject *>(kernel));
  for (const auto & kernel : _fv_kernels_system)
    kernels_to_print.push_back(dynamic_cast<MooseObject *>(kernel));

  console << ConsoleUtils::formatString(ConsoleUtils::mooseObjectVectorToString(kernels_to_print),
                                        "[DBG]")
          << std::endl;
}
