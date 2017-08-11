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

#ifndef EIGENVALUE_H
#define EIGENVALUE_H

#include "Steady.h"

class InputParameters;
class Eigenvalue;
class EigenProblem;

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<Eigenvalue>();

class Eigenvalue : public Steady
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  Eigenvalue(const InputParameters & parameters);

  virtual void execute() override;

protected:
  EigenProblem & _eigen_problem;
};

#endif // EIGENVALUE_H
