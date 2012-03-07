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
#include "Coupleable.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "PostprocessorInterface.h"
#include "TransientInterface.h"
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "MooseVariable.h"
#include "SubProblem.h"

// libMesh
#include "fe.h"
#include "quadrature.h"

class MooseMesh;
class Problem;
class SubProblem;


class ScalarKernel;

template<>
InputParameters validParams<ScalarKernel>();

class ScalarKernel :
  public MooseObject,
  public ScalarCoupleable,
  public SetupInterface,
  public FunctionInterface,
  public PostprocessorInterface,
  public TransientInterface
{
public:
  ScalarKernel(const std::string & name, InputParameters parameters);

  virtual void reinit() = 0;
  virtual void computeResidual() = 0;
  virtual void computeJacobian() = 0;

  /**
   * The variable that this kernel operates on.
   */
  MooseVariableScalar & variable() { return _var; }

  SubProblem & subProblem() { return _subproblem; }

  /**
   * Use this to enable/disable the constraint
   * @return true if the constrain is active
   */
  virtual bool isActive() { return true; }

protected:
  Problem & _problem;
  SubProblem & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;
  MooseVariableScalar & _var;                           ///< Scalar variable
  MooseMesh & _mesh;
  unsigned int _dim;

  unsigned int _i, _j;

  VariableValue & _u;                                    ///< Value(s) of the scalar variable
  VariableValue & _u_old;                                ///< Old value(s) of the scalar varaible
  VariableValue & _u_dot;
  VariableValue & _du_dot_du;

  // Single Instance Variables
  Real & _real_zero;
  MooseArray<Real> & _zero;
  MooseArray<RealGradient> & _grad_zero;
  MooseArray<RealTensor> & _second_zero;
};

#endif /* SCALARKERNEL_H */
