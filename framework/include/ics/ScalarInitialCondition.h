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
#include "ParallelUniqueId.h"
#include "InputParameters.h"
#include "Coupleable.h"
#include "DependencyResolverInterface.h"
#include "Reportable.h"

#include "libmesh/dense_vector.h"

// System includes
#include <string>

//forward declarations
class ScalarInitialCondition;
class SubProblem;
class SystemBase;
class Assembly;
class MooseVariableScalar;

template<>
InputParameters validParams<ScalarInitialCondition>();

/**
 * InitialConditions are objects that set the initial value of variables.
 */
class ScalarInitialCondition :
  public MooseObject,
  public ScalarCoupleable,
  public DependencyResolverInterface,
  public Reportable
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   */
  ScalarInitialCondition(const std::string & name, InputParameters parameters);

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
  SubProblem & _subproblem;
  SystemBase & _sys;
  THREAD_ID _tid;

  Assembly & _assembly;
  /// Scalar variable this initial condition works on
  MooseVariableScalar & _var;

  unsigned int _i;

  std::set<std::string> _depend_vars;
  std::set<std::string> _supplied_vars;
};

#endif //SCALARINITIALCONDITION_H
