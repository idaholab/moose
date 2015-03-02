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
#ifndef EXCEPTIONSTEADY_H
#define EXCEPTIONSTEADY_H

#include "Steady.h"

class ExceptionSteady;

template<>
InputParameters validParams<ExceptionSteady>();

/**
 * Test executioner to show exception handling
 */
class ExceptionSteady : public Steady
{
public:
  ExceptionSteady(const std::string & name, InputParameters parameters);
  virtual ~ExceptionSteady();

  /**
   * This will call solve() on the NonlinearSystem.
   */
  virtual void execute();
};

#endif /* EXCEPTIONSTEADY_H */
