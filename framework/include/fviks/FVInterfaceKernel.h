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
#include "FunctorInterface.h"

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
                          public FunctorInterface
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

  /**
   * Compute the residual on the supplied face
   */
  virtual void computeResidual(const FaceInfo & fi);

  /**
   * Compute the jacobian on the supplied face
   */
  virtual void computeJacobian(const FaceInfo & fi);

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
  void processResidual(Real resid, unsigned int var_num, bool neighbor);

  /**
   * Process the derivatives for the provided residual and dof index
   */
  void processDerivatives(const ADReal & resid, dof_id_type dof_index);

  /**
   * @return A structure that contains information about the face info element, face info, skewness
   * correction and element subdomain id for use with functors
   */
  Moose::ElemFromFaceArg elemFromFace(bool correct_skewness = false) const;

  /**
   * @return A structure that contains information about the face info neighbor, the face info,
   * skewness correction and the face info neighbor subdomain id for use with functors
   */
  Moose::ElemFromFaceArg neighborFromFace(bool correct_skenewss = false) const;

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

  /// The Assembly object
  Assembly & _assembly;

private:
  SystemBase & _sys;

  MooseVariableFV<Real> & _var1;
  MooseVariableFV<Real> & _var2;

  std::set<SubdomainID> _subdomain1;
  std::set<SubdomainID> _subdomain2;

  const MooseMesh & _mesh;
};
