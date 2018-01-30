//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BOUNDARYCONDITION_H
#define BOUNDARYCONDITION_H

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

// Forward declerations
class MooseVariable;
class MooseMesh;
class Problem;
class SubProblem;
class SystemBase;
class BoundaryCondition;
class Assembly;

template <>
InputParameters validParams<BoundaryCondition>();

/**
 * Base class for creating new types of boundary conditions.
 */
class BoundaryCondition : public MooseObject,
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
                          public MeshChangedInterface
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   * @param nodal Whether this BC is applied to nodes or not
   */
  BoundaryCondition(const InputParameters & parameters, bool nodal);

  /**
   * Gets the variable this BC is active on
   * @return the variable
   */
  MooseVariable & variable();

  /**
   * Get a reference to the subproblem
   * @return Reference to SubProblem
   */
  SubProblem & subProblem();

  /**
   * Hook for turning the boundary condition on and off.
   *
   * It is not safe to use variable values in this function, since (a) this is not called inside a
   * quadrature loop,
   * (b) reinit() is not called, thus the variables values are not computed.
   * NOTE: In NodalBC-derived classes, we can use the variable values, since renitNodeFace() was
   * called before calling
   * this method. However, one has to index into the values manually, i.e. not using _qp.
   * @return true if the boundary condition should be applied, otherwise false
   */
  virtual bool shouldApply();

protected:
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

  /// variable this BC works on
  MooseVariable & _var;

  /// Mesh this BC is defined on
  MooseMesh & _mesh;
};

#endif /* BOUNDARYCONDITION_H */
