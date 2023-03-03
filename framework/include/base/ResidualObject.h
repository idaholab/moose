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
 * This is the common base class for objects that give residual contributions.
 */
class ResidualObject : public MooseObject,
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
  ResidualObject(const InputParameters & parameters, bool nodal = false);

  /// Compute this object's contribution to the residual
  virtual void computeResidual() = 0;

  /// Compute this object's contribution to the diagonal Jacobian entries
  virtual void computeJacobian() = 0;

  /// Compute this object's contribution to the residual and Jacobian simultaneously
  virtual void computeResidualAndJacobian();

  /**
   * Computes this object's contribution to off-diagonal blocks of the system Jacobian matrix
   * @param jvar The number of the coupled variable. We pass the number of the coupled variable
   * instead of an actual variable object because we can query our system and obtain the variable
   * object associated with it. E.g. we need to make sure we are getting undisplaced variables if we
   * are working on the undisplaced mesh, and displaced variables if we are working on the displaced
   * mesh
   */
  virtual void computeOffDiagJacobian(unsigned int /*jvar*/) {}

  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar The number of the scalar variable
   */
  virtual void computeOffDiagJacobianScalar(unsigned int /*jvar*/) {}

  /**
   * Compute this object's contribution to the diagonal Jacobian entries
   * corresponding to nonlocal dofs of the variable
   */
  virtual void computeNonlocalJacobian() {}

  /**
   * Computes Jacobian entries corresponding to nonlocal dofs of the jvar
   */
  virtual void computeNonlocalOffDiagJacobian(unsigned int /* jvar */) {}

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
  virtual void precalculateResidual() {}
  virtual void precalculateJacobian() {}
  virtual void precalculateOffDiagJacobian(unsigned int /* jvar */) {}

  /**
   * Retrieve the variable object from our system associated with \p jvar_num
   */
  const MooseVariableFieldBase & getVariable(unsigned int jvar_num) const
  {
    return _sys.getVariable(_tid, jvar_num);
  }

protected:
  /// Reference to this kernel's SubProblem
  SubProblem & _subproblem;

  /// Reference to this kernel's FEProblemBase
  FEProblemBase & _fe_problem;

  /// Reference to the EquationSystem object
  SystemBase & _sys;

  /// The thread ID for this kernel
  THREAD_ID _tid;

  /// Reference to this Kernel's assembly object
  Assembly & _assembly;

  /// Reference to this Kernel's mesh object
  MooseMesh & _mesh;
};
