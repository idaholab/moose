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
#include "ScalarCoupleable.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "PostprocessorInterface.h"
#include "DependencyResolverInterface.h"
#include "TransientInterface.h"
#include "MooseVariableScalar.h"
#include "ZeroInterface.h"
#include "MeshChangedInterface.h"

// Forward declarations
class MooseMesh;
class SubProblem;
class Assembly;
class AuxScalarKernel;

template <>
InputParameters validParams<AuxScalarKernel>();

/**
 * Base class for making kernels that work on auxiliary scalar variables
 */
class AuxScalarKernel : public MooseObject,
                        public ScalarCoupleable,
                        public SetupInterface,
                        public FunctionInterface,
                        public UserObjectInterface,
                        public PostprocessorInterface,
                        public DependencyResolverInterface,
                        public TransientInterface,
                        public ZeroInterface,
                        public MeshChangedInterface
{
public:
  AuxScalarKernel(const InputParameters & parameters);

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

  virtual const std::set<std::string> & getRequestedItems() override;

  virtual const std::set<std::string> & getSuppliedItems() override;

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

  unsigned int _i;

  VariableValue & _u;
  VariableValue & _u_old;

  /// Depend AuxKernels
  std::set<std::string> _depend_vars;
  std::set<std::string> _supplied_vars;

  /**
   * Compute the value of this kernel.
   *
   * Each kernel must implement this.
   * @return The computed value
   */
  virtual Real computeValue() = 0;
};

#endif /* AUXSCALARKERNEL_H */
