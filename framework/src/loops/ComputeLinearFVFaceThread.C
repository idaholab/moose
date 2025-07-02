//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLinearFVFaceThread.h"
#include "LinearSystem.h"
#include "LinearFVKernel.h"
#include "LinearFVFluxKernel.h"
#include "FEProblemBase.h"

ComputeLinearFVFaceThread::ComputeLinearFVFaceThread(FEProblemBase & fe_problem,
                                                     const unsigned int system_num,
                                                     const Moose::FV::LinearFVComputationMode mode,
                                                     const std::set<TagID> & vector_tags,
                                                     const std::set<TagID> & matrix_tags)
  : _fe_problem(fe_problem),
    _system_number(system_num),
    _mode(mode),
    _vector_tags(vector_tags),
    _matrix_tags(matrix_tags),
    _system_contrib_objects_ready(false)
{
}

// Splitting Constructor
ComputeLinearFVFaceThread::ComputeLinearFVFaceThread(ComputeLinearFVFaceThread & x,
                                                     Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _system_number(x._system_number),
    _mode(x._mode),
    _vector_tags(x._vector_tags),
    _matrix_tags(x._matrix_tags),
    _system_contrib_objects_ready(x._system_contrib_objects_ready)
{
}

void
ComputeLinearFVFaceThread::operator()(const FaceInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  _old_subdomain = Moose::INVALID_BLOCK_ID;
  _old_neighbor_subdomain = Moose::INVALID_BLOCK_ID;

  setupSystemContributionObjects();
  printGeneralExecutionInformation();

  // Iterate over all the elements in the range
  for (const auto & face_info : range)
  {
    _subdomain = face_info->elem().subdomain_id();
    _neighbor_subdomain =
        face_info->neighborPtr() ? face_info->neighbor().subdomain_id() : _subdomain;
    if (_subdomain != _old_subdomain || _neighbor_subdomain != _old_neighbor_subdomain)
    {
      fetchBlockSystemContributionObjects();
      printBlockExecutionInformation();
    }

    const Real face_area = face_info->faceArea() * face_info->faceCoord();

    // Time to execute the kernels that contribute to the matrix and
    // right hand side
    for (auto & kernel : _fv_flux_kernels)
    {
      kernel->setupFaceData(face_info);
      kernel->setCurrentFaceArea(face_area);
      kernel->addMatrixContribution();
      kernel->addRightHandSideContribution();
    }
  }
}

void
ComputeLinearFVFaceThread::join(const ComputeLinearFVFaceThread & /*y*/)
{
}

void
ComputeLinearFVFaceThread::setupSystemContributionObjects()
{
  // First of all, we will collect the vectors and matrices for the assigned tags
  _base_query = _fe_problem.theWarehouse()
                    .query()
                    .template condition<AttribSysNum>(_system_number)
                    .template condition<AttribSystem>("LinearFVFluxKernel")
                    .template condition<AttribThread>(_tid)
                    .template condition<AttribMatrixTags>(_matrix_tags)
                    .template condition<AttribVectorTags>(_vector_tags);

  // We fetch all the available objects and make sure they are linked to the right
  // vectors and matrices
  std::vector<LinearFVFluxKernel *> kernels;
  _base_query.queryInto(kernels);

  // As a last step, we make sure the kernels know which vectors/matrices they need to contribute to
  for (auto & kernel : kernels)
    kernel->linkObjectsForContribution(_vector_tags, _matrix_tags);

  _system_contrib_objects_ready = true;
}

void
ComputeLinearFVFaceThread::fetchBlockSystemContributionObjects()
{
  mooseAssert(_system_contrib_objects_ready,
              "The system contribution objects need to be set up before we fetch the "
              "block-restricted objects!");

  _fv_flux_kernels.clear();

  if (_subdomain != _old_subdomain)
  {
    _base_query.condition<AttribSubdomains>(_subdomain).queryInto(_fv_flux_kernels_elem);
    _old_subdomain = _subdomain;
  }
  _fv_flux_kernels.insert(_fv_flux_kernels_elem.begin(), _fv_flux_kernels_elem.end());

  if (_neighbor_subdomain != _old_neighbor_subdomain)
  {
    _base_query.condition<AttribSubdomains>(_neighbor_subdomain)
        .queryInto(_fv_flux_kernels_neighbor);
    _old_neighbor_subdomain = _neighbor_subdomain;
  }
  _fv_flux_kernels.insert(_fv_flux_kernels_neighbor.begin(), _fv_flux_kernels_neighbor.end());
}

void
ComputeLinearFVFaceThread::printGeneralExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid))
    return;
  auto & console = _fe_problem.console();
  auto execute_on = _fe_problem.getCurrentExecuteOnFlag();
  console << "[DBG] Beginning linear finite volume flux objects loop on " << execute_on
          << std::endl;
  mooseDoOnce(console << "[DBG] Loop on faces (FaceInfo), objects ordered on each face: "
                      << std::endl;
              console << "[DBG] - linear finite volume flux kernels" << std::endl);
}

void
ComputeLinearFVFaceThread::printBlockExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid) || _fv_flux_kernels.empty())
    return;

  // Print the location of the execution
  auto & console = _fe_problem.console();
  console << "[DBG] Linear flux kernels on block "
          << _fe_problem.mesh().getSubdomainName(_subdomain);
  if (_neighbor_subdomain != Moose::INVALID_BLOCK_ID)
    console << " and neighbor " << _fe_problem.mesh().getSubdomainName(_neighbor_subdomain)
            << std::endl;
  else
    console << " with no neighbor block" << std::endl;

  // Print the list of objects
  std::vector<MooseObject *> kernels_to_print;
  for (const auto & kernel : _fv_flux_kernels)
    kernels_to_print.push_back(dynamic_cast<MooseObject *>(kernel));
  console << ConsoleUtils::formatString(ConsoleUtils::mooseObjectVectorToString(kernels_to_print),
                                        "[DBG]")
          << std::endl;
}
