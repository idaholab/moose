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

// Forward declerations
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<VectorValue<Real>> VectorMooseVariable;
class MooseMesh;
class Problem;
class SubProblem;
class SystemBase;

/**
 * Base class for creating new types of boundary conditions for linear FV systems.
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

  bool hasFaceSide(const FaceInfo & fi, bool fi_elem_side) const override;

  /**
   * Get a reference to the subproblem
   * @return Reference to SubProblem
   */
  const SubProblem & subProblem() const { return _subproblem; }

  /// Return the linear finite volume variable
  const MooseLinearVariableFV<Real> & variable() const { return *_var; }

  /**
   * Computes the boundary value of this object. This relies on the current solution field.
   * @param face_info Pointer to the FaceInfo object corresponding to the boundary face
   */
  virtual Real computeBoundaryValue() = 0;

  /**
   * Computes the normal gradient (often used in diffusion terms) on the boundary.
   * @param face_info Pointer to the FaceInfo object corresponding to the boundary face.
   */
  virtual Real computeBoundaryNormalGradient() = 0;

  /**
   * Computes the boundary value's contribution to the linear system matrix.
   * @param face_info Pointer to the FaceInfo object corresponding to the boundary face.
   */
  virtual Real computeBoundaryValueMatrixContribution() const = 0;

  /**
   * Computes the boundary value's contribution to the linear system right hand side.
   * @param face_info Pointer to the FaceInfo object corresponding to the boundary face.
   */
  virtual Real computeBoundaryValueRHSContribution() const = 0;

  /**
   * Computed the boundary gradient's contribution to the linear system matrix. Mostly used for
   * diffusion.
   * @param face_info Pointer to the FaceInfo object corresponding to the boundary face.
   */
  virtual Real computeBoundaryGradientMatrixContribution() const = 0;

  /**
   * Computed the boundary gradient's contribution to the linear system right hand side.
   * Mostly used for diffusion.
   * @param face_info Pointer to the FaceInfo object corresponding to the boundary face.
   */
  virtual Real computeBoundaryGradientRHSContribution() const = 0;

  /**
   * Check if the contributions to the right hand side and matrix already include the material
   * property multipler. For dirichlet boundary conditions this is false, but for flux boundary
   * conditions this can be true (like Neumann BC for diffusion problems).
   */
  bool includesMaterialPropertyMultiplier() const { return _includes_material_multiplier; }

  /**
   * Set function for the material property multiplier switch.
   */
  void setIncludesMaterialPropertyMultiplier(const bool new_setting)
  {
    _includes_material_multiplier = new_setting;
  }

  /// Set current face info
  void setCurrentFaceInfo(const FaceInfo * face_info, const FaceInfo::VarFaceNeighbors face_type)
  {
    mooseAssert(
        face_info,
        "The face info pointer should not be null when passing to the LinearFVBoundaryCondition!");
    _current_face_info = face_info;
    _current_face_type = face_type;
  }

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

  /// Thread id
  THREAD_ID _tid;

  /// Reference to SubProblem
  SubProblem & _subproblem;

  /// Mesh this BC is defined on
  MooseMesh & _mesh;

  /// Reference to the ruling finite volume problem
  FVProblemBase & _fv_problem;

  /// Pointer to the linear finite volume variable object
  MooseLinearVariableFV<Real> * _var;

  /// Reference to SystemBase
  SystemBase & _sys;

  /// Boolean to indicate if the boundary condition includes the material property multipliers ot not
  bool _includes_material_multiplier;

  /// Pointer to the face info we are operating on right now
  const FaceInfo * _current_face_info;

  /// Face ownership information for the current face
  FaceInfo::VarFaceNeighbors _current_face_type;
};
