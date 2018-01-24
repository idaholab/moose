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

#ifndef TESTSERIALIZEDSOLUTION_H
#define TESTSERIALIZEDSOLUTION_H

#include "GeneralPostprocessor.h"

class TestSerializedSolution;

template <>
InputParameters validParams<TestSerializedSolution>();

/**
 * A postprocessor for testing serialized solution vectors
 */
class TestSerializedSolution : public GeneralPostprocessor
{
public:
  TestSerializedSolution(const InputParameters & parameters);

  /**
   * Reset data
   */
  virtual void initialize();

  /**
   * Sum up all entries in the solution vector - verify the same answer on all processors
   */
  virtual void execute();

  /**
   * Return the summed value.
   */
  virtual Real getValue();

protected:
  /// The system to be tested
  SystemBase & _test_sys;

  /// Reference to the serialized solution for the test system
  NumericVector<Number> & _serialized_solution;

  /// Sum of all of the entries in the serialized solution vector
  Real _sum;
};

#endif
