//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

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
  static InputParameters validParams();

  Eigenvalue(const InputParameters & parameters);

  virtual void init() override;

  virtual void execute() override;

  /**
   * Here we scale the solution by the specified scalar and postprocessor value
   */
  virtual void postSolve() override;

protected:
  EigenProblem & _eigen_problem;
  /// Postprocessor value that scales solution when eigensolve is finished
  const PostprocessorValue * const _normalization;
};
