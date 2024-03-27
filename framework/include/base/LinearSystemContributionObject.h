//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "VectorPostprocessorInterface.h"
#include "RandomInterface.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"
#include "TaggingInterface.h"
#include "SystemBase.h"

#include "libmesh/linear_implicit_system.h"

class FEProblemBase;
class MooseMesh;
class SubProblem;
class MooseVariableBase;
class MooseVariableFieldBase;
class InputParameters;

/**
 * This is the common base class for objects that give contributions to a linear system
 */
class LinearSystemContributionObject : public MooseObject,
                                       public SetupInterface,
                                       public FunctionInterface,
                                       public UserObjectInterface,
                                       public TransientInterface,
                                       public PostprocessorInterface,
                                       public VectorPostprocessorInterface,
                                       public RandomInterface,
                                       public Restartable,
                                       public MeshChangedInterface,
                                       public TaggingInterface
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearSystemContributionObject(const InputParameters & parameters);

  /// Add this object's contribution to the system matrix
  virtual void addMatrixContribution() = 0;

  /// Add this object's contribution to the system right hand side
  virtual void addRightHandSideContribution() = 0;

  /// Returns base class reference of the variable that this object operates on.
  virtual const MooseVariableBase & variable() const = 0;

protected:
  /// Reference to this object's FEProblemBase
  FEProblemBase & _fe_problem;

  /// Reference to the system this object contributes to
  SystemBase & _sys;

  /// Reference to the libmesh linear system this object contributes to
  libMesh::LinearImplicitSystem & _linear_system;

  /// The thread ID for this object
  const THREAD_ID _tid;

  /// Reference to the mesh object
  MooseMesh & _mesh;
};
