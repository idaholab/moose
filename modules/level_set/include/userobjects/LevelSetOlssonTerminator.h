/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LEVELSETOLSSONTERMINATOR_H
#define LEVELSETOLSSONTERMINATOR_H

#include "GeneralUserObject.h"

//Forward Declarations
class LevelSetOlssonTerminator;
class Transient;

template<>
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

#endif //LEVELSETOLSSONTERMINATOR_H
