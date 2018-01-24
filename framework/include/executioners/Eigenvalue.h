//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
