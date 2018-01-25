//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LEVELSETREINITIALIZATIONPROBLEM_H
#define LEVELSETREINITIALIZATIONPROBLEM_H

#include "FEProblem.h"

class LevelSetReinitializationProblem;

template <>
InputParameters validParams<LevelSetReinitializationProblem>();

/**
 * A Problem object to perform level set equation reinitialization implementation, mainly
 * implementing
 * a method to reset the state of the simulation so a solve can be performed again.
 */
class LevelSetReinitializationProblem : public FEProblem
{
public:
  LevelSetReinitializationProblem(const InputParameters & parameters);

  /**
   * Resets the state of the simulation to allow for it to be re-executed.
   */
  void resetTime();
};

#endif
