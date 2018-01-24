//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SCALARINITIALCONDITION_H
#define SCALARINITIALCONDITION_H

#include "MooseObject.h"
#include "ScalarCoupleable.h"
#include "FunctionInterface.h"
#include "DependencyResolverInterface.h"

// forward declarations
class ScalarInitialCondition;
class FeProblem;
class SystemBase;
class Assembly;
class MooseVariableScalar;

namespace libMesh
{
template <typename T>
class DenseVector;
}

template <>
InputParameters validParams<ScalarInitialCondition>();

/**
 * InitialConditions are objects that set the initial value of variables.
 */
class ScalarInitialCondition : public MooseObject,
                               public ScalarCoupleable,
                               public FunctionInterface,
                               public DependencyResolverInterface
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  ScalarInitialCondition(const InputParameters & parameters);

  virtual ~ScalarInitialCondition();

  MooseVariableScalar & variable() { return _var; }

  /**
   * Compute the initial condition
   */
  virtual void compute(DenseVector<Number> & vals);

  /**
   * The value of the variable.
   *
   * This must be overridden by derived classes.
   */
  virtual Real value() = 0;

  virtual const std::set<std::string> & getRequestedItems();

  virtual const std::set<std::string> & getSuppliedItems();

protected:
  FEProblemBase & _fe_problem;
  SystemBase & _sys;
  THREAD_ID _tid;

  Assembly & _assembly;
  /// Time
  Real & _t;

  /// Scalar variable this initial condition works on
  MooseVariableScalar & _var;

  unsigned int _i;

  std::set<std::string> _depend_vars;
  std::set<std::string> _supplied_vars;
};

#endif // SCALARINITIALCONDITION_H
