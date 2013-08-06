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

#ifndef TESTCOPYINITIALSOLUTION_H
#define TESTCOPYINITIALSOLUTION_H

#include "GeneralPostprocessor.h"

class TestCopyInitialSolution;

template<>
InputParameters validParams<TestCopyInitialSolution>();

/**
 * A postprocessor for testing initial solution equality (see #1396)
 */
class TestCopyInitialSolution : public GeneralPostprocessor
{
public:
  TestCopyInitialSolution(const std::string & name, InputParameters parameters);
  virtual ~TestCopyInitialSolution();
  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

protected:
  bool _value;
};


#endif
