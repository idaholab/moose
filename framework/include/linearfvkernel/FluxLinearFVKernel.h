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
#include "ElemInfo.h"
#include "MooseVariableFV.h"
#include "MooseVariableInterface.h"
#include "FaceArgInterface.h"

class FluxLinearFVKernel : public LinearFVKernel, public FaceArgProducerInterface
{
public:
  static InputParameters validParams();
  FluxLinearFVKernel(const InputParameters & params);

  /// Compute this object's contribution to the system matrix
  virtual void addMatrixContribution() override;

  /// Compute this object's contribution to the right hand side
  virtual void addRightHandSideContribution() override;

  /// If the current element is defined on the elem side or neighbor side
  bool hasFaceSide(const FaceInfo & fi, bool fi_elem_side) const override;

  /// Set current face info
  void setCurrentFaceInfo(const FaceInfo * face_info)
  {
    _current_face_info = face_info;
    _current_face_type =
        _current_face_info->faceType(std::make_pair(_var->number(), _var->sys().number()));
  }

  /// Computes the system matrix contribution from an element side on an internal face
  virtual Real computeElemMatrixContribution() = 0;

  /// Computes the system matrix contribution from the neighbor side on an internal face
  virtual Real computeNeighborMatrixContribution() = 0;

  /// Computes the right hand side contribution from the element side on an internal face
  virtual Real computeElemRightHandSideContribution() = 0;

  /// Computes the right hand side contribution from the neighbor side on an internal face
  virtual Real computeNeighborRightHandSideContribution() = 0;

  /// Computes the system matrix contribution from a boundary face
  virtual Real computeBoundaryMatrixContribution() = 0;

  /// Computes the righth and side contribution from a boundary face
  virtual Real computeBoundaryRHSContribution() = 0;

protected:
  /**
   * Determine the single sided face argument when evaluating a functor on a face.
   * This is used to perform evaluations of material properties with the actual face values of
   * their dependences, rather than interpolate the material property to the boundary.
   * @param fi the FaceInfo for this face
   * @param limiter_type the limiter type, to be specified if more than the default average
   *        interpolation is required for the parameters of the functor
   * @param correct_skewness whether to perform skew correction at the face
   */
  Moose::FaceArg singleSidedFaceArg(
      const FaceInfo * fi = nullptr,
      Moose::FV::LimiterType limiter_type = Moose::FV::LimiterType::CentralDifference,
      bool correct_skewness = false) const;

  /// Pointer to the face info we are operating on right now
  const FaceInfo * _current_face_info;

  /// Face ownership information for the current face
  FaceInfo::VarFaceNeighbors _current_face_type;
};
