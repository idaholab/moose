//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

ComputeLinearFVFaceThread::ComputeLinearFVFaceThread(FEProblemBase & fe_problem,
                                                     const unsigned int linear_system_num,
                                                     const Moose::FV::LinearFVComputationMode mode,
                                                     const std::set<TagID> & tags)
  : _fe_problem(fe_problem), _linear_system_number(linear_system_num), _mode(mode), _tags(tags)
{
}

// Splitting Constructor
ComputeLinearFVFaceThread::ComputeLinearFVFaceThread(ComputeLinearFVFaceThread & x,
                                                     Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _linear_system_number(x._linear_system_number),
    _mode(x._mode),
    _tags(x._tags)
{
}

void
ComputeLinearFVFaceThread::operator()(const FaceInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  _old_subdomain = Moose::INVALID_BLOCK_ID;
  _old_neighbor_subdomain = Moose::INVALID_BLOCK_ID;

  // Iterate over all the elements in the range
  for (const auto & face_info : range)
  {
    _subdomain = face_info->elem().subdomain_id();
    _neighbor_subdomain =
        face_info->neighborPtr() ? face_info->neighbor().subdomain_id() : _subdomain;
    if (_subdomain != _old_subdomain || _neighbor_subdomain != _old_neighbor_subdomain)
      fetchSystemContributionObjects();

    for (auto kernel : _fv_flux_kernels)
    {
      kernel->setCurrentFaceInfo(face_info);
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
ComputeLinearFVFaceThread::join(const ComputeLinearFVFaceThread & /*y*/)
{
}

void
ComputeLinearFVFaceThread::fetchSystemContributionObjects()
{
  const auto system_number = _fe_problem.getLinearSystem(_linear_system_number).number();
  _fv_flux_kernels.clear();

  if (_subdomain != _old_subdomain)
  {
    _elem_fv_flux_kernels.clear();

    auto base_query = _fe_problem.theWarehouse()
                          .query()
                          .template condition<AttribSysNum>(system_number)
                          .template condition<AttribSubdomains>(_subdomain)
                          .template condition<AttribSystem>("LinearFVFluxKernel")
                          .template condition<AttribThread>(_tid);

    if (_mode == Moose::FV::LinearFVComputationMode::Matrix)
      base_query.condition<AttribMatrixTags>(_tags).queryInto(_elem_fv_flux_kernels);
    else
      base_query.condition<AttribVectorTags>(_tags).queryInto(_elem_fv_flux_kernels);
    _old_subdomain = _subdomain;
  }
  _fv_flux_kernels.insert(_elem_fv_flux_kernels.begin(), _elem_fv_flux_kernels.end());

  if (_neighbor_subdomain != _old_neighbor_subdomain)
  {
    _neighbor_fv_flux_kernels.clear();

    auto base_query = _fe_problem.theWarehouse()
                          .query()
                          .template condition<AttribSysNum>(system_number)
                          .template condition<AttribSubdomains>(_neighbor_subdomain)
                          .template condition<AttribSystem>("LinearFVFluxKernel")
                          .template condition<AttribThread>(_tid);

    if (_mode == Moose::FV::LinearFVComputationMode::Matrix)
      base_query.condition<AttribMatrixTags>(_tags).queryInto(_neighbor_fv_flux_kernels);
    else
      base_query.condition<AttribVectorTags>(_tags).queryInto(_neighbor_fv_flux_kernels);
    _old_neighbor_subdomain = _neighbor_subdomain;
  }
  _fv_flux_kernels.insert(_neighbor_fv_flux_kernels.begin(), _neighbor_fv_flux_kernels.end());
}
