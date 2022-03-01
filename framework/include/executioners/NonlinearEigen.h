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

class NonlinearEigen : public EigenExecutionerBase
{
public:
  static InputParameters validParams();

  NonlinearEigen(const InputParameters & parameters);

  virtual void init() override;

  virtual void execute() override;

  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

protected:
  virtual void takeStep();

  const unsigned int & _free_iter;
  const Real & _nl_abs_tol;
  const Real & _nl_rel_tol;
  const Real & _l_tol;
  const Real & _free_l_tol;

  bool _output_pi;
  bool _output_after_pi;
  bool _last_solve_converged;
};
