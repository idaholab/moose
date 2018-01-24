//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LEVELSETOLSSONTERMINATOR_H
#define LEVELSETOLSSONTERMINATOR_H

#include "GeneralUserObject.h"

// Forward Declarations
class LevelSetOlssonTerminator;
class Transient;

template <>
InputParameters validParams<LevelSetOlssonTerminator>();

/**
 * Terminates the solve based on the criteria defined in Olsson et. al. (2007).
 */
class LevelSetOlssonTerminator : public GeneralUserObject
{
public:
  LevelSetOlssonTerminator(const InputParameters & parameters);
  virtual void execute() override;
  virtual void initialize() override {}
  virtual void finalize() override {}

protected:
  /// The difference of current and old solutions
  NumericVector<Number> & _solution_diff;

  /// The steady-state convergence tolerance
  const Real & _tol;

  /// The required minimum number of timesteps
  const int & _min_t_steps;
};

#endif // LEVELSETOLSSONTERMINATOR_H
