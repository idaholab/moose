//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "libmesh/system.h"

class FEProblemBase;
class SystemBase;

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
   * @param system The system which contains variables that need gradients.
   * @param temporary_gradient Scratch storage for gradients being assembled.
   */
  ComputeLinearFVGreenGaussGradientFaceThread(
      FEProblemBase & fe_problem,
      SystemBase & system,
      std::vector<std::unique_ptr<NumericVector<Number>>> & temporary_gradient);

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

  /// The system wrapper this thread operates on.
  SystemBase & _system;

  /// Reference to the libMesh system backing the wrapper system.
  const libMesh::System & _libmesh_system;

  /// Global system number (the number of this system in the libmesh equation system)
  const unsigned int _system_number;

  /// Thread ID
  THREAD_ID _tid;

  /// Pointer to the current variable
  MooseLinearVariableFV<Real> * _current_var;

  /// Cache for the temporary gradient being built. It is needed because in certain scenarios the
  /// old gradient is used while assembling the replacement gradient.
  std::vector<std::unique_ptr<NumericVector<Number>>> & _temporary_gradient;
};
