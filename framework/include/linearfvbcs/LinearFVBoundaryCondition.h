//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE
#include "MooseObject.h"
#include "SetupInterface.h"
#include "ParallelUniqueId.h"
#include "FunctionInterface.h"
#include "DistributionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "VectorPostprocessorInterface.h"
#include "GeometricSearchInterface.h"
#include "BoundaryRestrictableRequired.h"
#include "MeshChangedInterface.h"
#include "TaggingInterface.h"
#include "MooseVariableDependencyInterface.h"
#include "NonADFunctorInterface.h"
#include "FaceArgInterface.h"
#include "MooseLinearVariableFV.h"

#include "libmesh/linear_implicit_system.h"

// Forward declerations
class MooseMesh;
class Problem;
class SubProblem;
class SystemBase;

/**
 * Base class for boundary conditions for linear FV systems.
 */
class LinearFVBoundaryCondition : public MooseObject,
                                  public BoundaryRestrictableRequired,
                                  public SetupInterface,
                                  public FunctionInterface,
                                  public DistributionInterface,
                                  public UserObjectInterface,
                                  public TransientInterface,
                                  public PostprocessorInterface,
                                  public VectorPostprocessorInterface,
                                  public GeometricSearchInterface,
                                  public MeshChangedInterface,
                                  public TaggingInterface,
                                  public MooseVariableInterface<Real>,
                                  public MooseVariableDependencyInterface,
                                  public NonADFunctorInterface,
                                  public FaceArgProducerInterface
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVBoundaryCondition(const InputParameters & parameters);

  static InputParameters validParams();

  virtual bool hasFaceSide(const FaceInfo & fi, bool fi_elem_side) const override;

  /**
   * Get a reference to the subproblem
   * @return Reference to SubProblem
   */
  const SubProblem & subProblem() const { return _subproblem; }

  /// Return the linear finite volume variable
  const MooseLinearVariableFV<Real> & variable() const { return _var; }

  /**
   * Computes the boundary value of this object. This relies on the current solution field.
   */
  virtual Real computeBoundaryValue() const = 0;

  /**
   * Computes the normal gradient (often used in diffusion terms) on the boundary.
   */
  virtual Real computeBoundaryNormalGradient() const = 0;

  /// Set current face info
  void setupFaceData(const FaceInfo * face_info, const FaceInfo::VarFaceNeighbors face_type);

  const FaceInfo * currentFaceInfo() const { return _current_face_info; }
  FaceInfo::VarFaceNeighbors currentFaceType() const { return _current_face_type; }

protected:
  /**
   * Compute the distance between the cell center and the face.
   */
  Real computeCellToFaceDistance() const;

  /**
   * Computes the vector connecting the cell and boundary face centers.
   * It is needed because sometimes boundaries can be assigned to internal faces as well.
   */
  RealVectorValue computeCellToFaceVector() const;

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

  /// Thread id
  const THREAD_ID _tid;

  /// Reference to SubProblem
  SubProblem & _subproblem;

  /// Mesh this BC is defined on
  MooseMesh & _mesh;

  /// Reference to the ruling finite volume problem
  FVProblemBase & _fv_problem;

  /// Reference to the linear finite volume variable object
  MooseLinearVariableFV<Real> & _var;

  /// Reference to system base class in MOOSE
  SystemBase & _sys;

  /// Reference to the libmesh linear system this object contributes to
  libMesh::LinearImplicitSystem & _linear_system;

  /// Pointer to the face info we are operating on right now
  const FaceInfo * _current_face_info;

  /// Face ownership information for the current face
  FaceInfo::VarFaceNeighbors _current_face_type;

  /// Cache for the variable number
  const unsigned int _var_num;

  /// Cache for the system number
  const unsigned int _sys_num;
};

inline void
LinearFVBoundaryCondition::setupFaceData(const FaceInfo * face_info,
                                         const FaceInfo::VarFaceNeighbors face_type)
{
  mooseAssert(
      face_info,
      "The face info pointer should not be null when passing to the LinearFVBoundaryCondition!");
  _current_face_info = face_info;
  _current_face_type = face_type;
}
