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

  std::vector<LinearFVFluxKernel *> kernels;
  auto base_query = _fe_problem.theWarehouse()
                        .query()
                        .template condition<AttribSysNum>(
                            _fe_problem.getLinearSystem(_linear_system_number).number())
                        .template condition<AttribSystem>("LinearFVFluxKernel")
                        .template condition<AttribThread>(_tid);

  if (_mode == Moose::FV::LinearFVComputationMode::Matrix)
    base_query.condition<AttribMatrixTags>(_tags).queryInto(kernels);
  else
    base_query.condition<AttribVectorTags>(_tags).queryInto(kernels);

  // Iterate over all the elements in the range
  for (const auto & face_info : range)
    for (auto kernel : kernels)
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

void
ComputeLinearFVFaceThread::join(const ComputeLinearFVFaceThread & /*y*/)
{
}
