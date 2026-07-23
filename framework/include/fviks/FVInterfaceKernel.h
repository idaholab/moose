//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TwoMaterialPropertyInterface.h"
#include "ADFunctorInterface.h"
#include "FVFaceResidualObject.h"
#include "FaceArgInterface.h"

#include <set>

class MooseMesh;
class SubProblem;
class FEProblemBase;
class SystemBase;
class Assembly;
template <typename>
class MooseVariableFV;

/**
 * Base class for creating kernels that interface physics between subdomains
 */
class FVInterfaceKernel : public MooseObject,
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
                          public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
                          public TwoMaterialPropertyInterface,
                          public ADFunctorInterface,
                          public FVFaceResidualObject,
                          public FaceArgProducerInterface
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  FVInterfaceKernel(const InputParameters & parameters);

  static InputParameters validParams();

  /**
   * Get a reference to the subproblem
   * @return Reference to SubProblem
   */
  const SubProblem & subProblem() const { return _subproblem; }

  void computeResidual(const FaceInfo & fi) override;
  void computeJacobian(const FaceInfo & fi) override;
  void computeResidualAndJacobian(const FaceInfo & fi) override;

  bool hasFaceSide(const FaceInfo & fi, bool fi_elem_side) const override;

protected:
  /**
   * @return The residual contribution for variable1 on subdomain1. The result will be multiplied
   * by the face area and the opposite contribution will be applied to variable2 on subdomain2
   * when both variables belong to the same nonlinear system.
   */
  virtual ADReal computeQpResidual() = 0;

  /**
   * @return The subdomains on the 1st side of this interface kernel. These are chosen by the user.
   * Variable 1 and any other data associated with the 1st side should exist on these subdomains
   */
  const std::set<SubdomainID> & sub1() const { return _subdomain1; }

  /**
   * @return The subdomains on the 2nd side of this interface kernel. These are chosen by the user.
   * Variable 2 and any other data associated with the 2nd side should exist on these subdomains
   */
  const std::set<SubdomainID> & sub2() const { return _subdomain2; }

  /**
   * @return Variable 1
   */
  const MooseVariableFV<Real> & var1() const { return _var1; }

  /**
   * @return Variable 2
   */
  const MooseVariableFV<Real> & var2() const { return _var2; }

  /**
   * @return The mesh this object lives on. Either undisplaced or displaced
   */
  const MooseMesh & mesh() const { return _mesh; }

  /**
   * setup data useful for this object
   */
  void setupData(const FaceInfo & fi);

  /**
   * Add a residual contribution to variable1 on subdomain1
   */
  void addResidualToVariable1(Real residual);

  /**
   * Add a residual contribution to variable2 on subdomain2
   */
  void addResidualToVariable2(Real residual);

  /**
   * Add a residual and its Jacobian contribution to variable1 on subdomain1
   */
  void addResidualAndJacobianToVariable1(const ADReal & residual);

  /**
   * Add a residual and its Jacobian contribution to variable2 on subdomain2
   */
  void addResidualAndJacobianToVariable2(const ADReal & residual);

  /**
   * @return The face normal oriented from subdomain1 toward subdomain2
   */
  Point normal() const;

  ///@{
  /**
   * @return The element on the corresponding user-defined side of the interface
   */
  const Elem & elem1() const;
  const Elem & elem2() const;
  ///@}

  ///@{
  /**
   * @return The element centroid on the corresponding user-defined side of the interface
   */
  const Point & centroid1() const;
  const Point & centroid2() const;
  ///@}

  ///@{
  /**
   * @return An element argument for the corresponding user-defined side of the interface
   */
  Moose::ElemArg elemArg1(bool correct_skewness = false) const;
  Moose::ElemArg elemArg2(bool correct_skewness = false) const;
  ///@}

  ///@{
  /**
   * @return A face argument explicitly restricted to the corresponding user-defined side
   */
  Moose::FaceArg
  faceArg1(Moose::FV::LimiterType limiter_type = Moose::FV::LimiterType::CentralDifference,
           bool correct_skewness = false,
           const Moose::StateArg * state_limiter = nullptr) const;
  Moose::FaceArg
  faceArg2(Moose::FV::LimiterType limiter_type = Moose::FV::LimiterType::CentralDifference,
           bool correct_skewness = false,
           const Moose::StateArg * state_limiter = nullptr) const;
  ///@}

  /**
   * Interpolate values from the two user-defined sides to the face
   */
  ADReal interpolateValue(Moose::FV::InterpMethod method,
                          const ADReal & value1,
                          const ADReal & value2) const;

  /// To be consistent with FE interfaces we introduce this quadrature point member. However, for FV
  /// calculations there should every only be one qudrature point and it should be located at the
  /// face centroid
  const unsigned int _qp = 0;

  /// The face that this object is currently operating on. Always set to something non-null before
  /// calling \p computeQpResidual
  const FaceInfo * _face_info = nullptr;

  /// Thread id
  const THREAD_ID _tid;

  /// The SubProblem
  SubProblem & _subproblem;

  MooseVariableFV<Real> & _var1;
  MooseVariableFV<Real> & _var2;

  /// The Assembly object
  Assembly & _assembly;

private:
  /**
   * Process the provided residual given \p var_num and whether this is on the neighbor side
   */
  void addResidual(Real residual, unsigned int var_num, bool neighbor);

  /**
   * Process the provided residual and its derivatives for a variable on a user-defined side
   */
  void addResidualAndJacobian(const ADReal & residual,
                              const MooseVariableFV<Real> & variable,
                              bool side_is_one);

  std::set<SubdomainID> _subdomain1;
  std::set<SubdomainID> _subdomain2;

  /// Whether the current FaceInfo element is on the user-defined first side
  bool _face_info_elem_on_side1 = false;

  const MooseMesh & _mesh;
};
