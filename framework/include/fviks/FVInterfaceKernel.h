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
#include "Restartable.h"
#include "MeshChangedInterface.h"
#include "TaggingInterface.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TwoMaterialPropertyInterface.h"

#include <set>

class MooseMesh;
class SubProblem;
class FEProblemBase;
class SystemBase;
class Assembly;

/**
 * Base class for creating new types of boundary conditions.
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
                          public Restartable,
                          public MeshChangedInterface,
                          public TaggingInterface,
                          public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
                          public TwoMaterialPropertyInterface
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   * @param nodal Whether this BC is applied to nodes or not
   */
  FVInterfaceKernel(const InputParameters & parameters);

  static InputParameters validParams();

  /**
   * Get a reference to the subproblem
   * @return Reference to SubProblem
   */
  const SubProblem & subProblem() const { return _subproblem; }

  void computeResidual(const FaceInfo & fi);
  void computeJacobian(const FaceInfo & fi);

protected:
  virtual ADReal computeQpResidual() = 0;
  const std::set<SubdomainID> & sub1() const { return _subdomain1; }
  const std::set<SubdomainID> & sub2() const { return _subdomain2; }
  const SystemBase & sys() const { return _sys; }
  bool elemIsOne() const { return _elem_is_one; }

  const unsigned int _qp = 0;

  ADRealVectorValue _normal;
  const FaceInfo * _face_info = nullptr;

  /// Thread id
  const THREAD_ID _tid;

private:
  SystemBase & _sys;
  SubProblem & _subproblem;

  MooseVariableField<Real> & _nonconst_var1;
  MooseVariableField<Real> & _nonconst_var2;

  std::set<SubdomainID> _subdomain1;
  std::set<SubdomainID> _subdomain2;

  Assembly & _assembly;

  /// whether the FaceInfo element corresponds to the first set of subdomains
  bool _elem_is_one;

protected:
  const MooseVariableField<Real> & _var1;
  const MooseVariableField<Real> & _var2;

  /// Mesh this interface kernel is defined on
  const MooseMesh & _mesh;
};
