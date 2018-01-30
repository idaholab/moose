//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SCALARKERNEL_H
#define SCALARKERNEL_H

#include "MooseObject.h"
#include "ScalarCoupleable.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "PostprocessorInterface.h"
#include "TransientInterface.h"
#include "MeshChangedInterface.h"
#include "VectorPostprocessorInterface.h"

// Forward declarations
class ScalarKernel;
class MooseMesh;
class Problem;
class SubProblem;
class Assembly;
class MooseVariableScalar;
class SubProblem;

template <>
InputParameters validParams<ScalarKernel>();

class ScalarKernel : public MooseObject,
                     public ScalarCoupleable,
                     public SetupInterface,
                     public FunctionInterface,
                     public UserObjectInterface,
                     public PostprocessorInterface,
                     public TransientInterface,
                     public MeshChangedInterface,
                     protected VectorPostprocessorInterface
{
public:
  ScalarKernel(const InputParameters & parameters);

  virtual void reinit() = 0;
  virtual void computeResidual() = 0;
  virtual void computeJacobian() = 0;
  virtual void computeOffDiagJacobian(unsigned int jvar);

  /**
   * The variable that this kernel operates on.
   */
  MooseVariableScalar & variable();

  SubProblem & subProblem();

  /**
   * Use this to enable/disable the constraint
   * @return true if the constrain is active
   */
  virtual bool isActive();

protected:
  SubProblem & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;
  /// Scalar variable
  MooseVariableScalar & _var;
  MooseMesh & _mesh;

  unsigned int _i, _j;

  /// Value(s) of the scalar variable
  VariableValue & _u;
  /// Old value(s) of the scalar variable
  VariableValue & _u_old;
  VariableValue & _u_dot;
  VariableValue & _du_dot_du;
};

#endif /* SCALARKERNEL_H */
