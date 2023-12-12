//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseMesh.h"

// libmesh
#include "libmesh/elem_range.h"
#include "libmesh/threads.h"

class FEProblemBase;
class LinearFVFluxKernel;

class ComputeLinearFVFaceThread
{
public:
  ComputeLinearFVFaceThread(FEProblemBase & fe_problem,
                            const unsigned int linear_system_num,
                            const Moose::FV::LinearFVComputationMode mode,
                            const std::set<TagID> & tags);

  // Splitting Constructor
  ComputeLinearFVFaceThread(ComputeLinearFVFaceThread & x, Threads::split split);
  using FaceInfoRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
  void operator()(const FaceInfoRange & range);
  void join(const ComputeLinearFVFaceThread & /*y*/);

  void fetchSystemContributionObjects();

protected:
  FEProblemBase & _fe_problem;
  const unsigned int _linear_system_number;
  const Moose::FV::LinearFVComputationMode _mode;
  const std::set<TagID> _tags;

  /// The subdomain for the current element
  SubdomainID _subdomain;

  /// The subdomain for the last element
  SubdomainID _old_subdomain;

  /// The subdomain for the current neighbor
  SubdomainID _neighbor_subdomain;

  /// The subdomain for the last neighbor
  SubdomainID _old_neighbor_subdomain;

  // Thread ID
  THREAD_ID _tid;

  /// LinearFVFluxKernels
  std::vector<LinearFVFluxKernel *> _elem_fv_flux_kernels;
  std::vector<LinearFVFluxKernel *> _neighbor_fv_flux_kernels;
  std::set<LinearFVFluxKernel *> _fv_flux_kernels;
};
