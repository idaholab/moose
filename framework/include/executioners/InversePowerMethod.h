//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "EigenExecutionerBase.h"

class InversePowerMethod : public EigenExecutionerBase
{
public:
  static InputParameters validParams();

  InversePowerMethod(const InputParameters & parameters);

  virtual void init() override;

  virtual void execute() override;

  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

protected:
  virtual void takeStep();

  /// name of the postprocessor for evaluating |x-xprevious|; empty means that no postprocessor is provided and power iteration will not check convergence based on it
  const PostprocessorName & _solution_diff_name;
  /// minimum number of power iterations
  const unsigned int & _min_iter;
  /// maximum number of power iterations
  const unsigned int & _max_iter;
  /// convergence tolerance on eigenvalue
  const Real & _eig_check_tol;
  /// convergence tolerance on solution difference
  const Real & _sol_check_tol;
  /// tolerance on each power iteration (always one nonlinear iteration)
  const Real & _l_tol;
  /// indicating if Chebyshev acceleration is turned on
  const bool & _cheb_on;
  /// flag to indicate if inverse power iteration converged
  bool _last_solve_converged;
};
