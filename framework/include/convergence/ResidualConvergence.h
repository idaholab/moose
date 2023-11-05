//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Convergence.h"

class ResidualConvergence : public Convergence
{
public:
  static InputParameters validParams();

  ResidualConvergence(const InputParameters & parameters);

  static InputParameters commonParams();

  Convergence::MooseAlgebraicConvergence checkAlgebraicConvergence(int it,
                        Real xnorm,
                        Real snorm,
                        Real fnorm) override;

protected:

  FEProblemBase & _fe_problem;

  bool _initialized;

  long int _nl_forced_its = 0;// the number of forced nonlinear iterations
  long int _nfuncs = 0;
  // Variables for the convergence criteria
  Real _atol; // absolute convergence tolerance
  Real _rtol; // relative convergence tolerance
  Real _stol; // convergence (step) tolerance in terms of the norm of the change in the
              // solution between steps
    
  Real _div_threshold = std::numeric_limits<Real>::max();      
  /// the absolute non linear divergence tolerance
  Real _nl_abs_div_tol = -1;
  Real _divtol; // relative divergence tolerance

  Real _nl_rel_tol;
  Real _nl_abs_tol;
  Real _nl_rel_step_tol;
  Real _nl_abs_step_tol;
  
  long int _nl_max_its;
  long int _nl_max_funcs;

  long int _maxit;  // maximum number of iterations
  long int _maxf;   // maximum number of function evaluations

  //Linear solver convergence criteria
  Real _l_tol;
  Real _l_abs_tol;
  long int _l_max_its;

  /// maximum number of ping-pong iterations
  unsigned int _n_nl_pingpong = 0;
  unsigned int _n_max_nl_pingpong = std::numeric_limits<unsigned int>::max();

};
