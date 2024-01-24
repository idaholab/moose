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

class ComputeLinearFVGreenGaussGradientThread
{
public:
  ComputeLinearFVGreenGaussGradientThread(FEProblemBase & fe_problem,
                                          const unsigned int linear_system_num);

  // Splitting Constructor
  ComputeLinearFVGreenGaussGradientThread(ComputeLinearFVGreenGaussGradientThread & x,
                                          Threads::split split);
  using FaceInfoRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
  void operator()(const FaceInfoRange & range);
  void join(const ComputeLinearFVGreenGaussGradientThread & /*y*/);

protected:
  void onInternalFace(const FaceInfo & face_info);
  void onBoundaryFace(const FaceInfo & face_info);

  FEProblemBase & _fe_problem;

  const unsigned int _dim;

  const unsigned int _linear_system_number;

  const libMesh::LinearImplicitSystem & _linear_system;

  // Thread ID
  THREAD_ID _tid;

  /// Pointer to the current variable
  MooseLinearVariableFV<Real> * _current_var;

  std::vector<std::unique_ptr<NumericVector<Number>>> _new_gradient;
};
