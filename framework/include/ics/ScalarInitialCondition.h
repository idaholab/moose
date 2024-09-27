//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialConditionInterface.h"
#include "MooseObject.h"
#include "ScalarCoupleable.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "DependencyResolverInterface.h"

class FeProblem;
class SystemBase;
class Assembly;
class MooseVariableScalar;

namespace libMesh
{
template <typename T>
class DenseVector;
}

/**
 * InitialConditions are objects that set the initial value of variables.
 */
class ScalarInitialCondition : public MooseObject,
                               public InitialConditionInterface,
                               public ScalarCoupleable,
                               public FunctionInterface,
                               public UserObjectInterface,
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

  static InputParameters validParams();

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

  /**
   * Gets called at the beginning of the simulation before this object is asked to do its job.
   * Note: This method is normally inherited from SetupInterface.  However in this case it makes
   * no sense to inherit the other virtuals available in that class so we are adding this virtual
   * directly to this class with out the extra inheritance.
   */
  virtual void initialSetup() {}

  virtual const std::set<std::string> & getRequestedItems();

  virtual const std::set<std::string> & getSuppliedItems();

protected:
  FEProblemBase & _fe_problem;
  SystemBase & _sys;
  THREAD_ID _tid;

  /// Time
  Real & _t;

  /// Scalar variable this initial condition works on
  MooseVariableScalar & _var;

  /// The finite element/volume assembly object
  Assembly & _assembly;

  unsigned int _i;

  std::set<std::string> _depend_vars;
  std::set<std::string> _supplied_vars;
};
