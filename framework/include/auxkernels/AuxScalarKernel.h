//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
