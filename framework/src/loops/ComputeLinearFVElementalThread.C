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
    _old_subdomain(Moose::INVALID_BLOCK_ID),
    _system_contrib_objects_ready(false)
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
    _old_subdomain(x._old_subdomain),
    _system_contrib_objects_ready(x._system_contrib_objects_ready)
{
}

void
ComputeLinearFVElementalThread::operator()(const ElemInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  _old_subdomain = Moose::INVALID_BLOCK_ID;

  setupSystemContributionObjects();
  printGeneralExecutionInformation();

  // Iterate over all the elements in the range
  for (const auto & elem_info : range)
  {
    _subdomain = elem_info->subdomain_id();
    if (_subdomain != _old_subdomain)
    {
      fetchBlockSystemContributionObjects();
      printBlockExecutionInformation();
    }

    const Real elem_volume = elem_info->volume() * elem_info->coordFactor();

    // Time to add the contributions from the kernels
    for (auto kernel : _fv_kernels)
    {
      kernel->setCurrentElemInfo(elem_info);
      kernel->setCurrentElemVolume(elem_volume);
      kernel->addMatrixContribution();
      kernel->addRightHandSideContribution();
    }
  }
}

void
ComputeLinearFVElementalThread::join(const ComputeLinearFVElementalThread & /*y*/)
{
}

void
ComputeLinearFVElementalThread::setupSystemContributionObjects()
{
  // First of all, we will collect the vectors and matrices for the assigned tags
  _base_query = _fe_problem.theWarehouse()
                    .query()
                    .template condition<AttribSysNum>(_system_number)
                    .template condition<AttribSystem>("LinearFVElementalKernel")
                    .template condition<AttribThread>(_tid)
                    .template condition<AttribMatrixTags>(_matrix_tags)
                    .template condition<AttribVectorTags>(_vector_tags);

  // We fetch all the available objects and make sure they are linked to the right
  // vectors and matrices
  std::vector<LinearFVElementalKernel *> kernels;
  _base_query.queryInto(kernels);

  // As a last step, we make sure the kernels know which vectors/matrices they need to contribute to
  for (auto & kernel : kernels)
    kernel->linkObjectsForContribution(_vector_tags, _matrix_tags);

  _system_contrib_objects_ready = true;
}

void
ComputeLinearFVElementalThread::fetchBlockSystemContributionObjects()
{
  mooseAssert(_system_contrib_objects_ready,
              "The system contribution objects need to be set up before we fetch the "
              "block-restricted objects!");

  // The base query already has the other conditions, here we just filter based on subdomain ID
  _base_query.template condition<AttribSubdomains>(_subdomain).queryInto(_fv_kernels);
  _old_subdomain = _subdomain;
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
  if (!_fe_problem.shouldPrintExecution(_tid) || _fv_kernels.empty())
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
