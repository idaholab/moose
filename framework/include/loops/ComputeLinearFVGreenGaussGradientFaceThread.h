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
#include "MooseLinearVariableFV.h"

// libmesh
#include "libmesh/elem_range.h"
#include "libmesh/threads.h"
#include "libmesh/linear_implicit_system.h"

class FEProblemBase;

/**
 * The gradient in a volume using Green Gauss theorem and a cell-centered finite-volume
 * approximation can be computed as follows:
 *
 * \nabla u \approx \frac{1}{V_C} \sum_f u_f\vec{S}_f,
 *
 * where V_C denotes the volume of the cell, f is a face iterator, while u_f and
 * \vec{S}_f are the face value of the variable and surface area vector (the product of the
 * surface area and normals), respectively. This object carries out the summation part over the
 * faces (\sum_f u_f\vec{S}_f).
 */
class ComputeLinearFVGreenGaussGradientFaceThread
{
public:
  /**
   * Class constructor.
   * @param fe_problem Reference to the problem
   * @param linear_system_num The number of the linear system which contains variables that need
   * gradients.
   */
  ComputeLinearFVGreenGaussGradientFaceThread(FEProblemBase & fe_problem,
                                              const unsigned int linear_system_num);

  /**
   * Splitting constructor.
   * @param x Reference to the other bodies
   * @param split The thread split
   */
  ComputeLinearFVGreenGaussGradientFaceThread(ComputeLinearFVGreenGaussGradientFaceThread & x,
                                              Threads::split split);
  using FaceInfoRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;

  /// Operator which is used to execute the thread over a certain iterator range.
  /// @param range The range of FaceInfos which should be computed.
  void operator()(const FaceInfoRange & range);

  /// Join threads at the end of the execution
  /// @param y Reference to the other bodies
  void join(const ComputeLinearFVGreenGaussGradientFaceThread & y);

protected:
  /// Reference to the problem
  FEProblemBase & _fe_problem;

  /// The dimension of the domain
  const unsigned int _dim;

  /// The number of the linear system on which this thread is acting.
  const unsigned int _linear_system_number;

  /// Reference to the linear system at libmesh level
  const libMesh::LinearImplicitSystem & _linear_system;

  /// Global system number (the number of this system in the libmesh equation system)
  const unsigned int _system_number;

  /// Thread ID
  THREAD_ID _tid;

  /// Pointer to the current variable
  MooseLinearVariableFV<Real> * _current_var;

  /// Cache for the new gradient which is being built. It is needed because in certain scenarios the
  /// old gradient is used for computing the new gradient.
  std::vector<std::unique_ptr<NumericVector<Number>>> & _new_gradient;
};
