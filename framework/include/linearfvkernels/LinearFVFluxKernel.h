//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVKernel.h"
#include "FaceArgInterface.h"

/**
 * Finite volume kernel that contributes approximations of discretized face flux terms to the matrix
 * and right hand side of a linear system.
 */
class LinearFVFluxKernel : public LinearFVKernel, public FaceArgProducerInterface
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVFluxKernel(const InputParameters & params);

  virtual void addMatrixContribution() override;

  virtual void addRightHandSideContribution() override;

  virtual bool hasFaceSide(const FaceInfo & fi, bool fi_elem_side) const override;

  /**
   * Set the current FaceInfo object
   * @param face_info The face info which will be used as current face info
   */
  virtual void setupFaceData(const FaceInfo * face_info);

  /**
   * Set the coordinate system specific face area for the assembly
   * @param area the face area
   */
  void setCurrentFaceArea(const Real area) { _current_face_area = area; };

  /// Computes the system matrix contribution from an element side on an internal face
  virtual Real computeElemMatrixContribution() = 0;

  /// Computes the system matrix contribution from the neighbor side on an internal face
  virtual Real computeNeighborMatrixContribution() = 0;

  /// Computes the right hand side contribution from the element side on an internal face
  virtual Real computeElemRightHandSideContribution() = 0;

  /// Computes the right hand side contribution from the neighbor side on an internal face
  virtual Real computeNeighborRightHandSideContribution() = 0;

  /**
   * Computes the matrix contribution from a boundary face
   * @param bc The boundary condition on the given face
   */
  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) = 0;

  /**
   * Computes the right hand side contribution from a boundary face
   * @param bc The boundary condition on the given face
   */
  virtual Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) = 0;

protected:
  /**
   * Determine the single sided face argument when evaluating a functor on a face.
   * @param fi the FaceInfo for this face
   * @param limiter_type the limiter type, to be specified if more than the default average
   *        interpolation is required for the parameters of the functor
   * @param correct_skewness whether to perform skew correction at the face
   */
  Moose::FaceArg singleSidedFaceArg(
      const FaceInfo * fi,
      Moose::FV::LimiterType limiter_type = Moose::FV::LimiterType::CentralDifference,
      bool correct_skewness = false) const;

  /// Pointer to the face info we are operating on right now
  const FaceInfo * _current_face_info;

  /// The current, coordinate system specific face area
  Real _current_face_area;

  /// Face ownership information for the current face
  FaceInfo::VarFaceNeighbors _current_face_type;

  /// If we already built the matrix contribution. This switch can be used to
  /// check if cached quantities are already available in the kernel.
  bool _cached_matrix_contribution;

  /// If we already built the right hand side contribution. This switch can be used to
  /// check if cached quantities are already available in the kernel.
  bool _cached_rhs_contribution;

  /// Whether to force execution of this kernel on all external boundaries
  const bool _force_boundary_execution;

  /// A vector of dof indices that describe where to add the
  /// matrix and right hand side batch contribution
  DenseVector<dof_id_type> _dof_indices;

  /// Cache for a batch of matrix contributions for faster assembly
  DenseMatrix<Real> _matrix_contribution;

  /// Cache for a batch of vector contributions for faster assembly
  DenseVector<Real> _rhs_contribution;
};
