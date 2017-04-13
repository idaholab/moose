/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSMULTIPHASEPROBLEM_H
#define RICHARDSMULTIPHASEPROBLEM_H

#include "FEProblem.h"

class RichardsMultiphaseProblem;

template <>
InputParameters validParams<RichardsMultiphaseProblem>();

/**
 * Allows a constraint u>=v to be enforced during
 * the nonlinear iteration process.  This is done
 * by modifying u (which is called bounded_var below)
 */
class RichardsMultiphaseProblem : public FEProblem
{
public:
  RichardsMultiphaseProblem(const InputParameters & params);
  virtual ~RichardsMultiphaseProblem();

  /**
   * extracts the moose variable numbers associated with bounded_var and lower_var
   */
  virtual void initialSetup();

  /// returns true, indicating that updateSolution should be run
  virtual bool shouldUpdateSolution();

  /**
   * Does the bounding by modifying vec_solution, and then ghosted_solution
   * @param vec_solution is the solution that Petsc says we should use.
   * @param ghosted_solution is a ghosted version of vec_solution.
   * @return true if vec_solution was changed at a node in order to respect the bounds
   */
  virtual bool updateSolution(NumericVector<Number> & vec_solution,
                              NumericVector<Number> & ghosted_solution);

protected:
  /// name of the bounded variable (this is the variable that gets altered to respect bounded_var > lower_var)
  NonlinearVariableName _bounded_var_name;

  /// name of the variable that acts as the lower bound to bounded_var
  NonlinearVariableName _lower_var_name;

  /// internal moose variable number associated with _bounded_var
  unsigned int _bounded_var_num;

  /// internal moose variable number associated with _lower_var
  unsigned int _lower_var_num;
};

#endif /* RICHARDSMULTIPHASEPROBLEM_H */
