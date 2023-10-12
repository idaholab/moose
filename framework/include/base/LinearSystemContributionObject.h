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
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
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

class FEProblemBase;
class MooseMesh;
class SubProblem;
class Assembly;
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
   * @param nodal Whether this object is applied to nodes or not
   */
  LinearSystemContributionObject(const InputParameters & parameters, bool nodal = false);

  /// Compute this object's contribution to the residual
  virtual void computeMatrixContribution() = 0;

  /// Compute this object's contribution to the diagonal Jacobian entries
  virtual void computeRightHandSideContribution() = 0;

  /**
   * Returns the variable that this object operates on.
   */
  virtual const MooseVariableBase & variable() const = 0;

  /**
   * Returns a reference to the SubProblem for which this Kernel is active
   */
  const SubProblem & subProblem() const { return _subproblem; }

  /**
   * Prepare shape functions
   * @param var_num The variable number whose shape functions should be prepared
   */
  virtual void prepareShapes(unsigned int var_num);

protected:
  /**
   * Retrieve the variable object from our system associated with \p jvar_num
   */
  const MooseVariableFieldBase & getVariable(unsigned int jvar_num) const
  {
    return _sys.getVariable(_tid, jvar_num);
  }

protected:
  /// Reference to this object's SubProblem
  SubProblem & _subproblem;

  /// Reference to this object's FEProblemBase
  FEProblemBase & _fe_problem;

  /// Reference to the system this object contributes to
  SystemBase & _sys;

  /// The thread ID for this object
  THREAD_ID _tid;

  /// Reference to the corresponding assembly object
  Assembly & _assembly;

  /// Reference to the mesh object
  MooseMesh & _mesh;
};
