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

#include "dense_vector.h"

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
  public MooseObject
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

protected:
  SubProblem & _subproblem;
  SystemBase & _sys;
  THREAD_ID _tid;

  Assembly & _assembly;
  /// Scalar variable this initial condition works on
  MooseVariableScalar & _var;

  unsigned int _i;
};

#endif //SCALARINITIALCONDITION_H
