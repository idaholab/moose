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

#ifndef TESTCONTROL_H
#define TESTCONTROL_H

// MOOSE includes
#include "Control.h"

// Forward declarations
class TestControl;

template <>
InputParameters validParams<TestControl>();

/**
 * A Control object for testing purposes
 */
class TestControl : public Control
{
public:
  TestControl(const InputParameters & parameters);
  virtual ~TestControl(){};
  virtual void execute();

private:
  /// The type of test to perform
  MooseEnum _test_type;
};

#endif // TESTCONTROL_H
