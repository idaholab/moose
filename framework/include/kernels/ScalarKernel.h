/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef SCALARKERNEL_H
#define SCALARKERNEL_H

#include "MooseObject.h"
#include "ScalarCoupleable.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "PostprocessorInterface.h"
#include "TransientInterface.h"
#include "ZeroInterface.h"
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
                     public ZeroInterface,
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
