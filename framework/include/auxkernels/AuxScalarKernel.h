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

#ifndef AUXSCALARKERNEL_H
#define AUXSCALARKERNEL_H

#include "MooseObject.h"
#include "Coupleable.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "PostprocessorInterface.h"
#include "TransientInterface.h"
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "SubProblem.h"
#include "Reportable.h"
#include "ZeroInterface.h"
// libMesh
#include "libmesh/fe.h"
#include "libmesh/quadrature.h"

class MooseMesh;
class Problem;
class SubProblem;


class AuxScalarKernel;

template<>
InputParameters validParams<AuxScalarKernel>();

/**
 * Base class for making kernels that work on auxiliary scalar variables
 */
class AuxScalarKernel :
  public MooseObject,
  public ScalarCoupleable,
  public SetupInterface,
  public FunctionInterface,
  public UserObjectInterface,
  public PostprocessorInterface,
  public TransientInterface,
  public Reportable,
  public ZeroInterface
{
public:
  AuxScalarKernel(const std::string & name, InputParameters parameters);

  virtual ~AuxScalarKernel();

  /**
   * Evaluate the kernel
   */
  virtual void compute();

  /**
   * The variable that this kernel operates on.
   */
  MooseVariableScalar & variable() { return _var; }

  SubProblem & subProblem() { return _subproblem; }

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
  MooseVariableScalar & _var;
  MooseMesh & _mesh;
  unsigned int _dim;

  unsigned int _i;

  VariableValue & _u;
  VariableValue & _u_old;

  /**
   * Compute the value of this kernel.
   *
   * Each kernel must implement this.
   * @return The computed value
   */
  virtual Real computeValue() = 0;
};

#endif /* AUXSCALARKERNEL_H */
