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

// Forward declerations
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<VectorValue<Real>> VectorMooseVariable;
class MooseMesh;
class Problem;
class SubProblem;
class SystemBase;
class Assembly;

/**
 * Base class for creating new types of boundary conditions.
 */
class FVBoundaryCondition : public MooseObject,
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
                            public MooseVariableDependencyInterface
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   * @param nodal Whether this BC is applied to nodes or not
   */
  FVBoundaryCondition(const InputParameters & parameters);

  static InputParameters validParams();

  /**
   * Get a reference to the subproblem
   * @return Reference to SubProblem
   */
  const SubProblem & subProblem() const { return _subproblem; }

protected:
  MooseVariableFV<Real> & _var;

  /// Reference to SubProblem
  SubProblem & _subproblem;

  /// Reference to FEProblemBase
  FEProblemBase & _fe_problem;

  /// Reference to SystemBase
  SystemBase & _sys;

  /// Thread id
  THREAD_ID _tid;

  /// Reference to assembly
  Assembly & _assembly;

  /// Mesh this BC is defined on
  MooseMesh & _mesh;
};
