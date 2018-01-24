//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef STEADY_H
#define STEADY_H

#include "Executioner.h"

// System includes
#include <string>

// Forward declarations
class InputParameters;
class Steady;
class FEProblemBase;

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<Steady>();

/**
 * Steady executioners usually only call "solve()" on the NonlinearSystem once.
 */
class Steady : public Executioner
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  Steady(const InputParameters & parameters);

  virtual void init() override;

  virtual void execute() override;

  virtual void checkIntegrity();

protected:
  FEProblemBase & _problem;

  int & _time_step;
  Real & _time;
};

#endif // STEADY_H
