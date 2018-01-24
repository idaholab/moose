//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TESTCOPYINITIALSOLUTION_H
#define TESTCOPYINITIALSOLUTION_H

#include "GeneralPostprocessor.h"

class TestCopyInitialSolution;

template <>
InputParameters validParams<TestCopyInitialSolution>();

/**
 * A postprocessor for testing initial solution equality (see #1396)
 */
class TestCopyInitialSolution : public GeneralPostprocessor
{
public:
  TestCopyInitialSolution(const InputParameters & parameters);
  virtual ~TestCopyInitialSolution();
  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

protected:
  bool _value;
};

#endif
