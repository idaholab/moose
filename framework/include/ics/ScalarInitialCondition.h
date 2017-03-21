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

#ifndef SCALARINITIALCONDITION_H
#define SCALARINITIALCONDITION_H

#include "MooseObject.h"
#include "ScalarCoupleable.h"
#include "FunctionInterface.h"
#include "DependencyResolverInterface.h"

#include "libmesh/dense_vector.h"

// forward declarations
class ScalarInitialCondition;
class FeProblem;
class SystemBase;
class Assembly;
class MooseVariableScalar;

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
