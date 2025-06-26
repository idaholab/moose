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
    _matrix_tags(matrix_tags)
{
}

// Splitting Constructor
ComputeLinearFVFaceThread::ComputeLinearFVFaceThread(ComputeLinearFVFaceThread & x,
                                                     Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _system_number(x._system_number),
    _mode(x._mode),
    _vector_tags(x._vector_tags),
    _matrix_tags(x._matrix_tags)
{
}

void
ComputeLinearFVFaceThread::operator()(const FaceInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  _old_subdomain = Moose::INVALID_BLOCK_ID;
  _old_neighbor_subdomain = Moose::INVALID_BLOCK_ID;

  printGeneralExecutionInformation();

  // Iterate over all the elements in the range
  for (const auto & face_info : range)
  {
    _subdomain = face_info->elem().subdomain_id();
    _neighbor_subdomain =
        face_info->neighborPtr() ? face_info->neighbor().subdomain_id() : _subdomain;
    if (_subdomain != _old_subdomain || _neighbor_subdomain != _old_neighbor_subdomain)
    {
      fetchSystemContributionObjects();
      printBlockExecutionInformation();
    }

    Real face_area = face_info->faceArea() * face_info->faceCoord();

    // First, we execute the kernels that contribute to both the matrix and
    // right hand side
    for (auto kernel : _fv_flux_kernels_system)
    {
      kernel->setupFaceData(face_info);
      kernel->setCurrentFaceArea(face_area);
      kernel->addMatrixContribution();
      kernel->addRightHandSideContribution();
    }
    // Second, we execute the kernels that contribute to the matrix only
    for (auto kernel : _fv_flux_kernels_matrix)
    {
      kernel->setupFaceData(face_info);
      kernel->setCurrentFaceArea(face_area);
      kernel->addMatrixContribution();
    }
    // Lastly, we execute the kernels that contribute to the right hand side only
    for (auto kernel : _fv_flux_kernels_rhs)
    {
      kernel->setupFaceData(face_info);
      kernel->setCurrentFaceArea(face_area);
      kernel->addRightHandSideContribution();
    }
  }
}

void
ComputeLinearFVFaceThread::join(const ComputeLinearFVFaceThread & /*y*/)
{
}

void
ComputeLinearFVFaceThread::fetchSystemContributionObjects()
{
  _fv_flux_kernels_matrix.clear();
  _fv_flux_kernels_rhs.clear();

  // For flux kernels we will have to check both sides of the face
  if (_subdomain != _old_subdomain)
  {
    auto base_query = _fe_problem.theWarehouse()
                          .query()
                          .template condition<AttribSysNum>(_system_number)
                          .template condition<AttribSubdomains>(_subdomain)
                          .template condition<AttribSystem>("LinearFVFluxKernel")
                          .template condition<AttribThread>(_tid);

    base_query.condition<AttribMatrixTags>(_matrix_tags).queryInto(_fv_flux_kernels_matrix_elem);
    base_query.condition<AttribVectorTags>(_vector_tags).queryInto(_fv_flux_kernels_rhs_elem);
    _old_subdomain = _subdomain;
  }
  _fv_flux_kernels_matrix.insert(_fv_flux_kernels_matrix.end(),
                                 _fv_flux_kernels_matrix_elem.begin(),
                                 _fv_flux_kernels_matrix_elem.end());
  _fv_flux_kernels_rhs.insert(_fv_flux_kernels_rhs.end(),
                              _fv_flux_kernels_rhs_elem.begin(),
                              _fv_flux_kernels_rhs_elem.end());

  if (_neighbor_subdomain != _old_neighbor_subdomain)
  {
    auto base_query = _fe_problem.theWarehouse()
                          .query()
                          .template condition<AttribSysNum>(_system_number)
                          .template condition<AttribSubdomains>(_neighbor_subdomain)
                          .template condition<AttribSystem>("LinearFVFluxKernel")
                          .template condition<AttribThread>(_tid);

    base_query.condition<AttribMatrixTags>(_matrix_tags)
        .queryInto(_fv_flux_kernels_matrix_neighbor);
    base_query.condition<AttribVectorTags>(_vector_tags).queryInto(_fv_flux_kernels_rhs_neighbor);
    _old_neighbor_subdomain = _neighbor_subdomain;
  }
  _fv_flux_kernels_matrix.insert(_fv_flux_kernels_matrix.end(),
                                 _fv_flux_kernels_matrix_neighbor.begin(),
                                 _fv_flux_kernels_matrix_neighbor.end());
  _fv_flux_kernels_rhs.insert(_fv_flux_kernels_rhs.end(),
                              _fv_flux_kernels_rhs_neighbor.begin(),
                              _fv_flux_kernels_rhs_neighbor.end());

  // This will remove the common elements and add them to the last argument
  MooseUtils::removeCommonSet(
      _fv_flux_kernels_matrix, _fv_flux_kernels_rhs, _fv_flux_kernels_system);
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
  if (!_fe_problem.shouldPrintExecution(_tid) ||
      (_fv_flux_kernels_matrix.empty() && _fv_flux_kernels_rhs.empty() &&
       _fv_flux_kernels_system.empty()))
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
  for (const auto & kernel : _fv_flux_kernels_matrix)
    kernels_to_print.push_back(dynamic_cast<MooseObject *>(kernel));
  for (const auto & kernel : _fv_flux_kernels_rhs)
    kernels_to_print.push_back(dynamic_cast<MooseObject *>(kernel));
  for (const auto & kernel : _fv_flux_kernels_system)
    kernels_to_print.push_back(dynamic_cast<MooseObject *>(kernel));
  console << ConsoleUtils::formatString(ConsoleUtils::mooseObjectVectorToString(kernels_to_print),
                                        "[DBG]")
          << std::endl;
}
