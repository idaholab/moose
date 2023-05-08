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
   * @return Interface residual terms. The result will be multiplied by the face area
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
   * @return The system associated with this object. Either an undisplaced or displaced nonlinear
   * system
   */
  const SystemBase & sys() const { return _sys; }

  /**
   * @return Whether the \p FaceInfo element is on the 1st side of the interface
   */
  virtual bool elemIsOne() const { return _elem_is_one; }

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
   * Process the provided residual given \p var_num and whether this is on the neighbor side
   */
  void addResidual(Real resid, unsigned int var_num, bool neighbor);

  using TaggingInterface::addJacobian;
  /**
   * Process the derivatives for the provided residual and dof index
   */
  void addJacobian(const ADReal & resid, dof_id_type dof_index, Real scaling_factor);

  /**
   * @return A structure that contains information about the face info element and skewness
   * correction for use with functors
   */
  Moose::ElemArg elemArg(bool correct_skewness = false) const;

  /**
   * @return A structure that contains information about the face info neighbor and skewness
   * correction for use with functors
   */
  Moose::ElemArg neighborArg(bool correct_skenewss = false) const;

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
      const MooseVariableFV<Real> & variable,
      const FaceInfo * fi = nullptr,
      Moose::FV::LimiterType limiter_type = Moose::FV::LimiterType::CentralDifference,
      bool correct_skewness = false) const;

  /// To be consistent with FE interfaces we introduce this quadrature point member. However, for FV
  /// calculations there should every only be one qudrature point and it should be located at the
  /// face centroid
  const unsigned int _qp = 0;

  /// The normal. This always points out of the \p FaceInfo element. Note, however that we multiply
  /// the result of \p computeQpResidual by -1 before summing into the \p FaceInfo neighbor residual
  /// such that the neighbor residual feels as if this data member is pointing out of the neighbor
  /// element
  ADRealVectorValue _normal;

  /// The face that this object is currently operating on. Always set to something non-null before
  /// calling \p computeQpResidual
  const FaceInfo * _face_info = nullptr;

  /// Thread id
  const THREAD_ID _tid;

  /// Whether the current element is associated with variable/subdomain 1 or 2
  bool _elem_is_one;

  /// The SubProblem
  SubProblem & _subproblem;

  /// the system object
  SystemBase & _sys;

  /// The Assembly object
  Assembly & _assembly;

private:
  MooseVariableFV<Real> & _var1;
  MooseVariableFV<Real> & _var2;

  std::set<SubdomainID> _subdomain1;
  std::set<SubdomainID> _subdomain2;

  const MooseMesh & _mesh;
};
