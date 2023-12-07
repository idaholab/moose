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

protected:
  FEProblemBase & _fe_problem;
  const unsigned int _linear_system_number;
  const Moose::FV::LinearFVComputationMode _mode;
  const std::set<TagID> _tags;
  THREAD_ID _tid;
};
